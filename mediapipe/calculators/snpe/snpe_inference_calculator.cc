#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/calculators/snpe/snpe_inference_calculator.pb.h"

namespace mediapipe {

class SnpeInferenceCalculator : public CalculatorBase {
public:
    static ::mediapipe::Status GetContract(CalculatorContract* cc);

    ::mediapipe::Status Open(CalculatorContext* cc) override;
    ::mediapipe::Status Process(CalculatorContext* cc) override;
    ::mediapipe::Status Close(CalculatorContext* cc) override;
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

    const auto& options = cc->Options<::mediapipe::SnpeInferenceCalculatorOptions>();

    return ::mediapipe::OkStatus();
}

::mediapipe::Status SnpeInferenceCalculator::Open(CalculatorContext* cc) {
    return ::mediapipe::OkStatus();
}

::mediapipe::Status SnpeInferenceCalculator::Process(CalculatorContext* cc) {
    const auto output_all_begin_id = cc->Outputs().BeginId();
    const auto output_all_end_id = cc->Outputs().EndId();
    for (auto id = cc->Inputs().BeginId(); id < cc->Inputs().EndId(); ++id) {
        if (id < output_all_begin_id || id >= output_all_end_id)
            continue;
        if (!cc->Inputs().Get(id).Value().IsEmpty())
            cc->Outputs().Get(id).AddPacket(cc->Inputs().Get(id).Value());
    }

    return ::mediapipe::OkStatus();
}

::mediapipe::Status SnpeInferenceCalculator::Close(CalculatorContext* cc) {
    return ::mediapipe::OkStatus();
}

} // namespace mediapipe
