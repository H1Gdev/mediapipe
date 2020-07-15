#include "mediapipe/framework/port.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/calculators/test/test_calculator.pb.h"

namespace mediapipe {

class TestCalculator : public CalculatorBase {
public:
    static ::mediapipe::Status GetContract(CalculatorContract* cc);

    ::mediapipe::Status Open(CalculatorContext* cc) override;
    ::mediapipe::Status Process(CalculatorContext* cc) override;
    ::mediapipe::Status Close(CalculatorContext* cc) override;
};

REGISTER_CALCULATOR(TestCalculator);

::mediapipe::Status TestCalculator::GetContract(CalculatorContract* cc) {
    DLOG(INFO) << "[S][" << cc->GetNodeName() << "]GetContract()";
    // Input/Output stream:
    // <TAG>:<INDEX>:<NAME>
    //
    // <TAG>   UPPER-CASE, underscore, number (default: "")
    // <INDEX> number(start with 0.)          (default: "0")
    // <NAME>  lower-case, underscore, number
    //
    // [All]
    DLOG(INFO) << "[Input stream]";
    DLOG(INFO) << ".NumEntries()=" << cc->Inputs().NumEntries();
    // CollectionItemId
    DLOG(INFO) << ".BeginId()=" << cc->Inputs().BeginId();
    DLOG(INFO) << ".EndId()=" << cc->Inputs().EndId();
    DLOG(INFO) << "[Output stream]";
    DLOG(INFO) << ".NumEntries()=" << cc->Outputs().NumEntries();
    DLOG(INFO) << ".BeginId()=" << cc->Outputs().BeginId();
    DLOG(INFO) << ".EndId()=" << cc->Outputs().EndId();
    // [Tag and Index]
    for (const auto& tag : cc->Inputs().GetTags()) {
        DLOG(INFO) << "[Input stream]\"" << tag << "\"";
        DLOG(INFO) << ".NumEntries(" << tag << ")=" << cc->Inputs().NumEntries(tag);
        DLOG(INFO) << ".BeginId(" << tag << ")=" << cc->Inputs().BeginId(tag);
        DLOG(INFO) << ".EndId(" << tag << ")=" << cc->Inputs().EndId(tag);
    }
    for (auto id = cc->Inputs().BeginId(); id < cc->Inputs().EndId(); ++id) {
        DLOG(INFO) << "CollectionItemId: " << id << " ->";
        const auto& tag_and_index = cc->Inputs().TagAndIndexFromId(id);
        DLOG(INFO) << "Tag: \"" << tag_and_index.first << "\" Index: " << tag_and_index.second;
        const auto& collection_item_id = cc->Inputs().GetId(tag_and_index.first, tag_and_index.second);
    }
    for (const auto& tag : cc->Outputs().GetTags()) {
        DLOG(INFO) << "[Output stream]\"" << tag << "\"";
        DLOG(INFO) << ".NumEntries(" << tag << ")=" << cc->Outputs().NumEntries(tag);
        DLOG(INFO) << ".BeginId(" << tag << ")=" << cc->Outputs().BeginId(tag);
        DLOG(INFO) << ".EndId(" << tag << ")=" << cc->Outputs().EndId(tag);
    }
    {
        // direct access.
        const auto tag = "TEST";
        const auto index = 0;
        auto id = cc->Inputs().GetId(tag, index);
        if (id.IsValid())
            DLOG(INFO) << "CollectionItemId: " << id << " -> " << "Tag: \"" << tag << "\" Index: " << index;
    }
    // Index(index) == Get("", index)
    // Tag(tag)     == Get(tag, 0)

    // Input/Output side packet:
    // <TAG>:<INDEX>:<NAME>
    //
    // - carried to all input_side_packet <NAME> in Graph.
    // - output_side_packet <NAME> should be unique in Graph.
    //
    // [All]
    DLOG(INFO) << "[Input side packet]";
    DLOG(INFO) << ".NumEntries()=" << cc->InputSidePackets().NumEntries();
    DLOG(INFO) << ".BeginId()=" << cc->InputSidePackets().BeginId();
    DLOG(INFO) << ".EndId()=" << cc->InputSidePackets().EndId();
    DLOG(INFO) << "[Output side packet]";
    DLOG(INFO) << ".NumEntries()=" << cc->OutputSidePackets().NumEntries();
    DLOG(INFO) << ".BeginId()=" << cc->OutputSidePackets().BeginId();
    DLOG(INFO) << ".EndId()=" << cc->OutputSidePackets().EndId();
    // [Tag and Index]
    for (const auto& tag : cc->InputSidePackets().GetTags()) {
        DLOG(INFO) << "[Input side packet]\"" << tag << "\"";
        DLOG(INFO) << ".NumEntries(" << tag << ")=" << cc->InputSidePackets().NumEntries(tag);
        DLOG(INFO) << ".BeginId(" << tag << ")=" << cc->InputSidePackets().BeginId(tag);
        DLOG(INFO) << ".EndId(" << tag << ")=" << cc->InputSidePackets().EndId(tag);
    }
    for (const auto& tag : cc->OutputSidePackets().GetTags()) {
        DLOG(INFO) << "[Output side packet]\"" << tag << "\"";
        DLOG(INFO) << ".NumEntries(" << tag << ")=" << cc->OutputSidePackets().NumEntries(tag);
        DLOG(INFO) << ".BeginId(" << tag << ")=" << cc->OutputSidePackets().BeginId(tag);
        DLOG(INFO) << ".EndId(" << tag << ")=" << cc->OutputSidePackets().EndId(tag);
    }

    // set PacketType.
    // input stream
    for (auto& input : cc->Inputs()) { // use iterator.
        input.SetAny();
    }
    const auto input_all_num_entries = cc->Inputs().NumEntries();
    const auto input_all_begin_id = cc->Inputs().BeginId();
    const auto input_all_end_id = cc->Inputs().EndId();
    // output stream
    for (auto id = cc->Outputs().BeginId(); id < cc->Outputs().EndId(); ++id) { // use CollectionItemId.
        auto& output = cc->Outputs().Get(id);
        if (id >= input_all_begin_id && id < input_all_end_id)
            output.SetSameAs(&cc->Inputs().Get(id));
        else
            output.SetAny();
    }
    // input side packet
    for (auto& input : cc->InputSidePackets()) {
        input.SetAny();
    }
    // output side packet
    for (auto& output : cc->OutputSidePackets()) {
        output.Set<std::string>();
    }

    // Options
    // --
    // node {
    //   calculator: "TestCalculator"
    //   node_options: {
    //     [type.googleapis.com/mediapipe.TestCalculatorOptions] {
    //       test_name: "test"
    //     }
    //   }
    // }
    // --
    const auto& options = cc->Options<::mediapipe::TestCalculatorOptions>();
    // [Protocol Buffer API]
    // https://developers.google.com/protocol-buffers/docs/cpptutorial#the-protocol-buffer-api
    const auto& test_name = options.test_name();
    if (options.has_test_id())
        const auto& test_id = options.test_id();
    else
        const auto& test_id_default = options.test_id();

    ::mediapipe::TestCalculatorOptions::TestEnum test_enum_value = ::mediapipe::TestCalculatorOptions::ONE;
    if (options.has_test_enum())
        const auto& test_enum = options.test_enum();
    else
        const auto& test_enum_default = options.test_enum();

    if (options.test_values_size() > 0)
        const auto& head = options.test_values(0);
    for (const auto& test_value : options.test_values())
        DLOG(INFO) << test_value;

    // Platform
#ifdef MEDIAPIPE_MOBILE
    LOG(INFO) << "Platform is Mobile.";
#endif
#ifdef MEDIAPIPE_ANDROID
    LOG(INFO) << "Platform is Android.";
#endif
#ifdef MEDIAPIPE_IOS
    LOG(INFO) << "Platform is iOS.";
#endif
#ifdef MEDIAPIPE_MEDIAPIPE_OSX
    LOG(INFO) << "Platform is OS X.";
#endif
#ifdef MEDIAPIPE_DISABLE_GL_COMPUTE
    LOG(INFO) << "Disable GL.";
#endif

    DLOG(INFO) << "[E][" << cc->GetNodeName() << "]GetContract()";
    return ::mediapipe::OkStatus();
}

::mediapipe::Status TestCalculator::Open(CalculatorContext* cc) {
    DLOG(INFO) << "[S][" << cc->NodeName() << "]Open() " << cc->InputTimestamp();
    DLOG(INFO) << "[CalculatorContext]";
    DLOG(INFO) << ".CalculatorType()=" << cc->CalculatorType();
    DLOG(INFO) << ".NodeId()=" << cc->NodeId();

    // input side packet
    for (auto& input : cc->InputSidePackets()) {
        if (input.ValidateAsType<std::string>().ok())
            LOG(INFO) << "[IN]Side Packet: " << input.Get<std::string>();
    }
    // output side packet
    for (auto& output : cc->OutputSidePackets()) {
        output.Set(MakePacket<std::string>("Output Side Packet..."));
    }

    DLOG(INFO) << "[E][" << cc->NodeName() << "]Open()";
    // if error occurs, graph terminates.
    return ::mediapipe::OkStatus();
}

::mediapipe::Status TestCalculator::Process(CalculatorContext* cc) {
    // if there is no input(s) from input_stream(s), Process() will not be called and there will be no output(s) to output_stream(s).
    DLOG(INFO) << "[S][" << cc->NodeName() << "]Process() " << cc->InputTimestamp();

    // InputStreamShard and OutputStreamShard.
    const auto output_all_begin_id = cc->Outputs().BeginId();
    const auto output_all_end_id = cc->Outputs().EndId();
    for (auto id = cc->Inputs().BeginId(); id < cc->Inputs().EndId(); ++id) {
        const auto& input_packet = cc->Inputs().Get(id).Value();
        DLOG(INFO) << "[IN][" << id << "]Packet: " << input_packet;
        if (id < output_all_begin_id || id >= output_all_end_id)
            continue;
        if (input_packet.IsEmpty())
            continue;

        DLOG(INFO) << cc->Inputs().Get(id).Name() << " -> " << cc->Outputs().Get(id).Name();
#if 1
        // for std::string type.
        if (input_packet.ValidateAsType<std::string>().ok()) {
            const auto& input_data = input_packet.Get<std::string>();
            DLOG(INFO) << "[IN][" << id << "]Data: " << input_data;
            // create output data.
            std::unique_ptr<std::string> output_data(new std::string(input_data));
            cc->Outputs().Get(id).Add(output_data.release(), cc->Inputs().Get(id).Value().Timestamp());
        } else
#endif
            // go through Packet.
            cc->Outputs().Get(id).AddPacket(cc->Inputs().Get(id).Value());
    }

    DLOG(INFO) << "[E][" << cc->NodeName() << "]Process() " << cc->InputTimestamp();
    // if error occurs, framework calls Close() and graph terminates.
    return ::mediapipe::OkStatus();
}

::mediapipe::Status TestCalculator::Close(CalculatorContext* cc) {
    DLOG(INFO) << "[S][" << cc->NodeName() << "]Close() " << cc->InputTimestamp();
    DLOG(INFO) << "[E][" << cc->NodeName() << "]Close()";
    // if Open() was succeeded, framework calls this.
    return ::mediapipe::OkStatus();
}

} // namespace mediapipe
