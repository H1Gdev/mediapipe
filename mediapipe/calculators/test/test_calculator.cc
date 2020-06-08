#include "mediapipe/framework/calculator_framework.h"

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

    return ::mediapipe::OkStatus();
}

::mediapipe::Status TestCalculator::Open(CalculatorContext* cc) {
    // if error occurs, graph terminates.
    return ::mediapipe::OkStatus();
}

::mediapipe::Status TestCalculator::Process(CalculatorContext* cc) {
    // if there is no input(s) from input_stream(s), Process() will not be called and there will be no output(s) to output_stream(s).

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

    // if error occurs, framework calls Close() and graph terminates.
    return ::mediapipe::OkStatus();
}

::mediapipe::Status TestCalculator::Close(CalculatorContext* cc) {
    // if Open() was succeeded, framework calls this.
    return ::mediapipe::OkStatus();
}

} // namespace mediapipe
