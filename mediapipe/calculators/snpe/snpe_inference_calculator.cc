#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/util/resource_util.h"

#include "mediapipe/calculators/snpe/snpe_inference_calculator.pb.h"

#include "absl/memory/memory.h"
#include "absl/strings/str_split.h"

#include "DiagLog/IDiagLog.hpp"
#include "DlContainer/IDlContainer.hpp"
#include "DlSystem/DlEnums.hpp"
#include "DlSystem/DlError.hpp"
#include "DlSystem/ITensorFactory.hpp"
#include "DlSystem/IUserBuffer.hpp"
#include "DlSystem/IUserBufferFactory.hpp"
#include "DlSystem/RuntimeList.hpp"
#include "DlSystem/UserBufferMap.hpp"
#include "SNPE/SNPEBuilder.hpp"
#include "SNPE/SNPEFactory.hpp"

namespace mediapipe {

class SnpeInferenceCalculator : public CalculatorBase {
public:
    static ::mediapipe::Status GetContract(CalculatorContract* cc);

    ::mediapipe::Status Open(CalculatorContext* cc) override;
    ::mediapipe::Status Process(CalculatorContext* cc) override;
    ::mediapipe::Status Close(CalculatorContext* cc) override;

private:
    std::map<std::string, std::pair<std::string, int>> input_tensor_map_;
    std::map<std::string, std::pair<std::string, int>> output_tensor_map_;

    std::unique_ptr<zdl::SNPE::SNPE> snpe_;
    zdl::DiagLog::IDiagLog* logger_;
};

REGISTER_CALCULATOR(SnpeInferenceCalculator);

::mediapipe::Status SnpeInferenceCalculator::GetContract(CalculatorContract* cc) {
    // input
    const auto input_index = cc->Inputs().NumEntries();
    for (auto i = 0; i < input_index; ++i)
        cc->Inputs().Index(i).SetAny();
    // output
    const auto output_index = cc->Outputs().NumEntries();
    for (auto i = 0; i < output_index; ++i)
        if (i < input_index)
            cc->Outputs().Index(i).SetSameAs(&cc->Inputs().Index(i));
        else
            cc->Outputs().Index(i).SetAny();

    LOG(INFO) << "SNPE version: " << zdl::SNPE::SNPEFactory::getLibraryVersion().asString();

    const auto& options = cc->Options<::mediapipe::SnpeInferenceCalculatorOptions>();

    // Runtime
    if (options.has_runtime()) {
        auto& runtime_str = options.runtime();
        zdl::DlSystem::Runtime_t runtime = zdl::DlSystem::RuntimeList::stringToRuntime(runtime_str.c_str());
        if (runtime == zdl::DlSystem::Runtime_t::UNSET)
            return ::mediapipe::InvalidArgumentError("invalid runtime: " + runtime_str);
        else if (!zdl::SNPE::SNPEFactory::isRuntimeAvailable(runtime))
            return ::mediapipe::UnavailableError("unavailable runtime: " + runtime_str);
    } else {
        std::vector<std::string> runtime_str_list = absl::StrSplit(options.runtime_list(), ",", absl::SkipWhitespace());
        for (auto& runtime_str : runtime_str_list) {
            zdl::DlSystem::Runtime_t runtime = zdl::DlSystem::RuntimeList::stringToRuntime(runtime_str.c_str());
            if (runtime == zdl::DlSystem::Runtime_t::UNSET)
                return ::mediapipe::InvalidArgumentError("invalid runtime: " + runtime_str);
        }
    }

    return ::mediapipe::OkStatus();
}

::mediapipe::Status SnpeInferenceCalculator::Open(CalculatorContext* cc) {
    const auto& options = cc->Options<::mediapipe::SnpeInferenceCalculatorOptions>();

    // Runtime
    zdl::DlSystem::RuntimeList runtime_list;
    if (options.has_runtime()) {
        zdl::DlSystem::Runtime_t runtime = zdl::DlSystem::RuntimeList::stringToRuntime(options.runtime().c_str());
        if (!runtime_list.add(runtime))
            return ::mediapipe::InternalError(zdl::DlSystem::getLastErrorString());
    } else {
        std::vector<std::string> runtime_str_list = absl::StrSplit(options.runtime_list(), ",", absl::SkipWhitespace());
        for (auto& runtime_str : runtime_str_list) {
            zdl::DlSystem::Runtime_t runtime = zdl::DlSystem::RuntimeList::stringToRuntime(runtime_str.c_str());
            if (!runtime_list.add(runtime))
                return ::mediapipe::InternalError(zdl::DlSystem::getLastErrorString());
        }
    }

    // Container
    // [Android]if not absolute path, search only filename from assets.
    ASSIGN_OR_RETURN(std::string model_file_path, ::mediapipe::PathToResourceAsFile(options.model_file_path()));
    LOG(INFO) << "model_file_path: " << model_file_path;

    std::unique_ptr<zdl::DlContainer::IDlContainer> container = zdl::DlContainer::IDlContainer::open(model_file_path);
    if (!container)
        return ::mediapipe::InvalidArgumentError("can not open model: " + options.model_file_path());

    // PlatformConfig
    zdl::DlSystem::PlatformConfig platformConfig;// TODO: has field ?

    zdl::DlSystem::StringList output_tensor_names;
    for (const auto& output_tensor : options.output_tensor_map())
        output_tensor_names.append(output_tensor.tensor().c_str());

#ifndef MEDIAPIPE_ANDROID
    if (options.buffer_type() == ::mediapipe::SnpeInferenceCalculatorOptions::USER_FLOAT_GPU)
        return ::mediapipe::InvalidArgumentError("can not use GPU buffer.");
#endif
    const auto use_user_supplied_buffers = options.buffer_type() == ::mediapipe::SnpeInferenceCalculatorOptions::USER_FLOAT_CPU ||
                                           options.buffer_type() == ::mediapipe::SnpeInferenceCalculatorOptions::USER_FLOAT_GPU ||
                                           options.buffer_type() == ::mediapipe::SnpeInferenceCalculatorOptions::USER_TF8;

    // SNPE
    zdl::SNPE::SNPEBuilder builder(container.get());

    // "Network Resizing" dynamically if needed.
    // https://developer.qualcomm.com/docs/snpe/network_resize.html

    snpe_ = builder
            .setPerformanceProfile(static_cast<zdl::DlSystem::PerformanceProfile_t>(options.performance_profile()))
            .setProfilingLevel(static_cast<zdl::DlSystem::ProfilingLevel_t>(options.profiling_level()))
            .setExecutionPriorityHint(static_cast<zdl::DlSystem::ExecutionPriorityHint_t>(options.execution_priority_hint()))
#if 0
            // TODO: What is the difference between Layers and Tensors ? (LayerName + ":0" == TensorName ?)
            .setOutputLayers(output_tensor_names)
#endif
            .setOutputTensors(output_tensor_names)
            .setUseUserSuppliedBuffers(use_user_supplied_buffers)
            .setDebugMode(options.debug_mode())
            .setCPUFallbackMode(options.cpu_fallback_mode())
            .setRuntimeProcessorOrder(runtime_list)
            .setInitCacheMode(options.init_cache_mode())
            .setPlatformConfig(platformConfig)
            .build();
    if (!snpe_)
        return ::mediapipe::InternalError("error while building SNPE.");

    LOG(INFO) << "model version: " << snpe_->getModelVersion();

    if (options.init_cache_mode() && !container->save(model_file_path))
        LOG(WARNING) << "failed to save container into archive.";

    if (options.has_log_directory_path()) {
        auto logger = snpe_->getDiagLogInterface();
        if (!logger)
            return ::mediapipe::InvalidArgumentError("can not save diagnostic log.");
        logger_ = *logger;
        auto logging_options = logger_->getOptions();
#if 0
        // TODO: no errors occur with this value. but log is not output correctly...
        logging_options.DiagLogMask = "DNN_RUNTIME=ON";
#endif
        logging_options.LogFileDirectory = options.log_directory_path();
        if(!logger_->setOptions(logging_options))
            return ::mediapipe::InvalidArgumentError("can not set log directory: " + options.log_directory_path());
        if(!logger->start())
            return ::mediapipe::InvalidArgumentError("can not start log. please check permissions.");
#if 0
        // TODO: no errors occur with this value. but log is not output correctly...
        if (!logger->setDiagLogMask("DNN_RUNTIME=ON"))
            return ::mediapipe::InvalidArgumentError("can not set log mask.");
#endif
    }

    // Input
    const zdl::DlSystem::StringList input_names = snpe_->getInputTensorNames();
    for (const auto& input_tensor : options.input_tensor_map()) {
        if(std::find(input_names.begin(), input_names.end(), input_tensor.tensor()) == input_names.end())
            return ::mediapipe::InvalidArgumentError("not found input tensor: " + input_tensor.tensor());
        for (auto id = cc->Inputs().BeginId(); id < cc->Inputs().EndId(); ++id)
            if (cc->Inputs().Get(id).Name() == input_tensor.stream()) {
                input_tensor_map_.insert(std::make_pair(input_tensor.tensor(), cc->Inputs().TagAndIndexFromId(id)));
                break;
            }

        auto optional = snpe_->getInputDimensions(input_tensor.tensor().c_str());
        if (!optional)
            return ::mediapipe::InvalidArgumentError("not exists input tensor: " + input_tensor.tensor());
        const auto& tensor_shape = *optional;
        std::vector<zdl::DlSystem::Dimension> dimensions(tensor_shape.getDimensions(), tensor_shape.getDimensions() + tensor_shape.rank());
        LOG(INFO) << "input \"" << input_tensor.tensor() << "\": [" << absl::StrJoin(dimensions, ", ") << "] (batch size: " << dimensions.at(0) << ")";
    }
    // Output
    const zdl::DlSystem::StringList output_names = snpe_->getOutputTensorNames();
    for (const auto& output_tensor : options.output_tensor_map()) {
        if(std::find(output_names.begin(), output_names.end(), output_tensor.tensor()) == output_names.end())
            return ::mediapipe::InvalidArgumentError("not found output tensor: " + output_tensor.tensor());
        for (auto id = cc->Outputs().BeginId(); id < cc->Outputs().EndId(); ++id)
            if (cc->Outputs().Get(id).Name() == output_tensor.stream()) {
                output_tensor_map_.insert(std::make_pair(output_tensor.tensor(), cc->Outputs().TagAndIndexFromId(id)));
                break;
            }
    }

    return ::mediapipe::OkStatus();
}

::mediapipe::Status SnpeInferenceCalculator::Process(CalculatorContext* cc) {
    const auto& options = cc->Options<::mediapipe::SnpeInferenceCalculatorOptions>();

    if (options.buffer_type() == ::mediapipe::SnpeInferenceCalculatorOptions::ITENSOR) {
        // Map
        zdl::DlSystem::TensorMap input_map, output_map;

        // for manage.
        std::vector<std::unique_ptr<zdl::DlSystem::ITensor>> input_tensors;

        // Input
        for (const auto& input_tensor : options.input_tensor_map()) {
            const zdl::DlSystem::TensorShape tensor_shape = snpe_->getInputDimensions(input_tensor.tensor().c_str());

            std::vector<zdl::DlSystem::Dimension> dimensions(tensor_shape.getDimensions(), tensor_shape.getDimensions() + tensor_shape.rank());
            LOG(INFO) << "input: \"" << input_tensor.stream() << "\" -> \"" << input_tensor.tensor() << "\": [" << absl::StrJoin(dimensions, ", ") << "]";

            const auto& tag_index = input_tensor_map_.at(input_tensor.tensor());
            const auto& input_stream = cc->Inputs().Get(tag_index.first, tag_index.second);
            const auto& input_packet = input_stream.Value();
            if (input_packet.IsEmpty())
                return ::mediapipe::InvalidArgumentError("input stream packet is empty: " + input_stream.Name());

            // Tensor
            auto input = zdl::SNPE::SNPEFactory::getTensorFactory().createTensor(tensor_shape);

            if (input_packet.ValidateAsType<std::vector<float>>().ok()) {
                const auto& data = input_packet.Get<std::vector<float>>();
                std::copy(data.cbegin(), data.cend(), input->begin());
            }
            input_map.add(input_tensor.tensor().c_str(), input.get());
            input_tensors.push_back(std::move(input));
        }

        // Execute
        if (!snpe_->execute(input_map, output_map))
            return ::mediapipe::InternalError("error while executing SNPE.");

        // Output
        for (const auto& output_tensor : options.output_tensor_map()) {
            // Tensor
            const auto output = output_map.getTensor(output_tensor.tensor().c_str());
            const zdl::DlSystem::TensorShape tensor_shape = output->getShape();

            std::vector<zdl::DlSystem::Dimension> dimensions(tensor_shape.getDimensions(), tensor_shape.getDimensions() + tensor_shape.rank());
            LOG(INFO) << "output: \"" << output_tensor.tensor() << "\": [" << absl::StrJoin(dimensions, ", ") << "] -> \"" << output_tensor.stream() << "\"";

            auto data = absl::make_unique<std::vector<float>>(output->getSize());
            std::copy(output->cbegin(), output->cend(), data->begin());

            const auto& tag_index = output_tensor_map_.at(output_tensor.tensor());
            cc->Outputs().Get(tag_index.first, tag_index.second).Add(data.release(), cc->InputTimestamp());
        }
    } else {
        // Map
        zdl::DlSystem::UserBufferMap input_map, output_map;
        auto& user_buffer_factory = zdl::SNPE::SNPEFactory::getUserBufferFactory();

        // for manage.
        std::vector<std::unique_ptr<zdl::DlSystem::IUserBuffer>> input_user_buffers;
        std::vector<std::unique_ptr<zdl::DlSystem::IUserBuffer>> output_user_buffers;

        if (options.buffer_type() == ::mediapipe::SnpeInferenceCalculatorOptions::USER_FLOAT_CPU) {
            zdl::DlSystem::UserBufferEncodingFloat encoding;

            // Input
            for (const auto& input_tensor : options.input_tensor_map()) {
                const auto& tag_index = input_tensor_map_.at(input_tensor.tensor());

                const auto& input_stream = cc->Inputs().Get(tag_index.first, tag_index.second);
                const auto& input_packet = input_stream.Value();
                if (input_packet.IsEmpty())
                    return ::mediapipe::InvalidArgumentError("input stream packet is empty: " + input_stream.Name());

                auto buffer_attributes = snpe_->getInputOutputBufferAttributes(input_tensor.tensor().c_str());
                if (!buffer_attributes)
                    return ::mediapipe::InvalidArgumentError("error obtaining attributes for input tensor: " + input_tensor.tensor());
                const auto& tensor_shape = (*buffer_attributes)->getDims();

                std::vector<zdl::DlSystem::Dimension> dimensions(tensor_shape.getDimensions(), tensor_shape.getDimensions() + tensor_shape.rank());
                LOG(INFO) << "input: \"" << input_tensor.stream() << "\" -> \"" << input_tensor.tensor() << "\": [" << absl::StrJoin(dimensions, ", ") << "]";

                if (input_packet.ValidateAsType<std::vector<float>>().ok()) {
                    const auto& data = input_packet.Get<std::vector<float>>();

                    std::vector<zdl::DlSystem::Dimension> strides(tensor_shape.rank());
                    strides[strides.size() - 1] = sizeof(float);
                    for (auto i = tensor_shape.rank() - 1; i > 0; i--)
                        strides[i - 1] = strides[i] * tensor_shape[i];

                    // UserBuffer
                    auto input = user_buffer_factory.createUserBuffer(const_cast<float*>(data.data()), data.size() * sizeof(float), strides, &encoding);
                    input_map.add(input_tensor.tensor().c_str(), input.get());
                    input_user_buffers.push_back(std::move(input));
                }
            }

            // Output
            for (const auto& output_tensor : options.output_tensor_map()) {
                const auto& tag_index = output_tensor_map_.at(output_tensor.tensor());

                auto buffer_attributes = snpe_->getInputOutputBufferAttributes(output_tensor.tensor().c_str());
                if (!buffer_attributes)
                    return ::mediapipe::InvalidArgumentError("error obtaining attributes for output tensor: " + output_tensor.tensor());
                const auto& tensor_shape = (*buffer_attributes)->getDims();

                std::vector<zdl::DlSystem::Dimension> dimensions(tensor_shape.getDimensions(), tensor_shape.getDimensions() + tensor_shape.rank());
                LOG(INFO) << "output: \"" << output_tensor.tensor() << "\": [" << absl::StrJoin(dimensions, ", ") << "] -> \"" << output_tensor.stream() << "\"";

                size_t size = std::accumulate(dimensions.begin(), dimensions.end(), 1, [](zdl::DlSystem::Dimension init, zdl::DlSystem::Dimension v) { return init * v; });
                auto data = absl::make_unique<std::vector<float>>(size);

                std::vector<zdl::DlSystem::Dimension> strides(tensor_shape.rank());
                strides[strides.size() - 1] = sizeof(float);
                for (auto i = tensor_shape.rank() - 1; i > 0; i--)
                    strides[i - 1] = strides[i] * tensor_shape[i];

                // UserBuffer
                auto output = user_buffer_factory.createUserBuffer(data->data(), data->size() * sizeof(float), strides, &encoding);
                output_map.add(output_tensor.tensor().c_str(), output.get());
                output_user_buffers.push_back(std::move(output));

                cc->Outputs().Get(tag_index.first, tag_index.second).Add(data.release(), cc->InputTimestamp());
            }

            // Execute
            if (!snpe_->execute(input_map, output_map))
                return ::mediapipe::InternalError("error while executing SNPE.");
        } else if (options.buffer_type() == ::mediapipe::SnpeInferenceCalculatorOptions::USER_FLOAT_GPU) {
            // TODO
        } else if (options.buffer_type() == ::mediapipe::SnpeInferenceCalculatorOptions::USER_TF8) {
            // TODO: support string for mediapipe.
            zdl::DlSystem::UserBufferEncodingTfN encoding(0, 1.f);

            // Input
            for (const auto& input_tensor : options.input_tensor_map()) {
                const auto& tag_index = input_tensor_map_.at(input_tensor.tensor());

                const auto& input_stream = cc->Inputs().Get(tag_index.first, tag_index.second);
                const auto& input_packet = input_stream.Value();
                if (input_packet.IsEmpty())
                    return ::mediapipe::InvalidArgumentError("input stream packet is empty: " + input_stream.Name());

                auto buffer_attributes = snpe_->getInputOutputBufferAttributes(input_tensor.tensor().c_str());
                if (!buffer_attributes)
                    return ::mediapipe::InvalidArgumentError("error obtaining attributes for input tensor: " + input_tensor.tensor());
                const auto& tensor_shape = (*buffer_attributes)->getDims();

                std::vector<zdl::DlSystem::Dimension> dimensions(tensor_shape.getDimensions(), tensor_shape.getDimensions() + tensor_shape.rank());
                LOG(INFO) << "input: \"" << input_tensor.stream() << "\" -> \"" << input_tensor.tensor() << "\": [" << absl::StrJoin(dimensions, ", ") << "]";

                if (input_packet.ValidateAsType<std::vector<uint8_t>>().ok()) {
                    const auto& data = input_packet.Get<std::vector<uint8_t>>();

                    std::vector<zdl::DlSystem::Dimension> strides(tensor_shape.rank());
                    strides[strides.size() - 1] = sizeof(uint8_t);
                    for (auto i = tensor_shape.rank() - 1; i > 0; i--)
                        strides[i - 1] = strides[i] * tensor_shape[i];

                    // UserBuffer
                    auto input = user_buffer_factory.createUserBuffer(const_cast<uint8_t*>(data.data()), data.size() * sizeof(uint8_t), strides, &encoding);
                    input_map.add(input_tensor.tensor().c_str(), input.get());
                    input_user_buffers.push_back(std::move(input));
                }
            }

            // Output
            for (const auto& output_tensor : options.output_tensor_map()) {
                const auto& tag_index = output_tensor_map_.at(output_tensor.tensor());

                auto buffer_attributes = snpe_->getInputOutputBufferAttributes(output_tensor.tensor().c_str());
                if (!buffer_attributes)
                    return ::mediapipe::InvalidArgumentError("error obtaining attributes for output tensor: " + output_tensor.tensor());
                const auto& tensor_shape = (*buffer_attributes)->getDims();

                std::vector<zdl::DlSystem::Dimension> dimensions(tensor_shape.getDimensions(), tensor_shape.getDimensions() + tensor_shape.rank());
                LOG(INFO) << "output: \"" << output_tensor.tensor() << "\": [" << absl::StrJoin(dimensions, ", ") << "] -> \"" << output_tensor.stream() << "\"";

                size_t size = std::accumulate(dimensions.begin(), dimensions.end(), 1, [](zdl::DlSystem::Dimension init, zdl::DlSystem::Dimension v) { return init * v; });
                auto data = absl::make_unique<std::vector<uint8_t>>(size);

                std::vector<zdl::DlSystem::Dimension> strides(tensor_shape.rank());
                strides[strides.size() - 1] = sizeof(uint8_t);
                for (auto i = tensor_shape.rank() - 1; i > 0; i--)
                    strides[i - 1] = strides[i] * tensor_shape[i];

                // UserBuffer
                auto output = user_buffer_factory.createUserBuffer(data->data(), data->size() * sizeof(uint8_t), strides, &encoding);
                output_map.add(output_tensor.tensor().c_str(), output.get());
                output_user_buffers.push_back(std::move(output));

                cc->Outputs().Get(tag_index.first, tag_index.second).Add(data.release(), cc->InputTimestamp());
            }

            // Execute
            if (!snpe_->execute(input_map, output_map))
                return ::mediapipe::InternalError("error while executing SNPE.");
        }
    }

    return ::mediapipe::OkStatus();
}

::mediapipe::Status SnpeInferenceCalculator::Close(CalculatorContext* cc) {
    if (logger_ && !logger_->stop())
        LOG(WARNING) << "can not stop log.";
    snpe_.reset();

    return ::mediapipe::OkStatus();
}

} // namespace mediapipe
