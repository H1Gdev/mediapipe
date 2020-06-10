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

    // set PacketType.
    // input
    for (auto& input : cc->Inputs()) { // use iterator.
        input.SetAny();
    }
    const auto input_all_num_entries = cc->Inputs().NumEntries();
    const auto input_all_begin_id = cc->Inputs().BeginId();
    const auto input_all_end_id = cc->Inputs().EndId();
    // output
    for (auto id = cc->Outputs().BeginId(); id < cc->Outputs().EndId(); ++id) { // use CollectionItemId.
        auto& output = cc->Outputs().Get(id);
        if (id >= input_all_begin_id && id < input_all_end_id)
            output.SetSameAs(&cc->Inputs().Get(id));
        else
            output.SetAny();
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

    DLOG(INFO) << "[E][" << cc->GetNodeName() << "]GetContract()";
    return ::mediapipe::OkStatus();
}

::mediapipe::Status TestCalculator::Open(CalculatorContext* cc) {
    DLOG(INFO) << "[S][" << cc->NodeName() << "]Open() " << cc->InputTimestamp();
    DLOG(INFO) << "[CalculatorContext]";
    DLOG(INFO) << ".CalculatorType()=" << cc->CalculatorType();
    DLOG(INFO) << ".NodeId()=" << cc->NodeId();
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
        if (id < output_all_begin_id || id >= output_all_end_id)
            continue;
        if (!cc->Inputs().Get(id).Value().IsEmpty())
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
