#include "mediapipe/framework/input_stream_handler.h"

#include "mediapipe/framework/stream_handler/test/test_input_stream_handler.pb.h"

namespace mediapipe {

// Input Stream Handler
class TestInputStreamHandler : public InputStreamHandler {
public:
    TestInputStreamHandler() = delete;
    TestInputStreamHandler(
        std::shared_ptr<tool::TagMap> tag_map,
        CalculatorContextManager* calculator_context_manager,
        const MediaPipeOptions& options,
        bool calculator_run_in_parallel);

protected:
    NodeReadiness GetNodeReadiness(Timestamp* min_stream_timestamp) override;
    void FillInputSet(Timestamp input_timestamp, InputStreamShardSet* input_set) override;
};

REGISTER_INPUT_STREAM_HANDLER(TestInputStreamHandler);

TestInputStreamHandler::TestInputStreamHandler(
    std::shared_ptr<tool::TagMap> tag_map,
    CalculatorContextManager* calculator_context_manager,
    const MediaPipeOptions& options,
    bool calculator_run_in_parallel)
    : InputStreamHandler(std::move(tag_map), calculator_context_manager, options, calculator_run_in_parallel) {
    DLOG(INFO) << "[S]TestInputStreamHandler()";
    // options
    if (options.HasExtension(TestInputStreamHandlerOptions::ext)) {
        const auto& stream_handler_options = options.GetExtension(TestInputStreamHandlerOptions::ext);
        if (stream_handler_options.has_name())
            DLOG(INFO) << "name is " << stream_handler_options.name();
    }
    DLOG(INFO) << "[E]TestInputStreamHandler()";
}

NodeReadiness TestInputStreamHandler::GetNodeReadiness(Timestamp* min_stream_timestamp) {
    DLOG(INFO) << "[S]GetNodeReadiness()";
    // check
    DCHECK(min_stream_timestamp);

    DLOG(INFO) << "*min_stream_timestamp=" << *min_stream_timestamp;
#if 0
    DLOG(INFO) << "[E]GetNodeReadiness()=NodeReadiness::kNotReady";
    return NodeReadiness::kNotReady;
#elif 0
    *min_stream_timestamp = Timestamp::Done();
    DLOG(INFO) << "[E]GetNodeReadiness()=NodeReadiness::kReadyForClose";
    return NodeReadiness::kReadyForClose;
#elif 0
    *min_stream_timestamp = Timestamp(0);
    DLOG(INFO) << "[E]GetNodeReadiness()=NodeReadiness::kReadyForProcess";
    return NodeReadiness::kReadyForProcess;
#else
    auto min_packet = Timestamp::Done();
    auto min_bound = Timestamp::Done();
    for (const auto& input_stream_manager : input_stream_managers_) {
        DLOG(INFO) << "Name()=" << input_stream_manager->Name();

        bool empty;
        const auto stream_timestamp = input_stream_manager->MinTimestampOrBound(&empty);
        DLOG(INFO) << "MinTimestampOrBound(" << empty << ")=" << stream_timestamp;
        if (empty) {
            min_bound = std::min(min_bound, stream_timestamp);
        } else {
            min_packet = std::min(min_packet, stream_timestamp);
        }
        *min_stream_timestamp = std::min(min_packet, min_bound);

        if (*min_stream_timestamp == Timestamp::Done()) {
            DLOG(INFO) << "[E]GetNodeReadiness()=NodeReadiness::kReadyForClose";
            return NodeReadiness::kReadyForClose;
        }
    }
    DLOG(INFO) << "min_packet=" << min_packet;
    DLOG(INFO) << "min_bound=" << min_bound;
    DLOG(INFO) << "*min_stream_timestamp=" << *min_stream_timestamp;

    if (ProcessTimestampBounds()) {
        const auto stream_timestamp = std::min(min_packet, min_bound.PreviousAllowedInStream());
        if (stream_timestamp > Timestamp::Unstarted()) {
            *min_stream_timestamp = stream_timestamp;
            DLOG(INFO) << "*min_stream_timestamp=" << *min_stream_timestamp;
            DLOG(INFO) << "[E]GetNodeReadiness()=NodeReadiness::kReadyForProcess";
            return NodeReadiness::kReadyForProcess;
        }
    } else if (min_bound > min_packet) {
        DLOG(INFO) << "[E]GetNodeReadiness()=NodeReadiness::kReadyForProcess";
        return NodeReadiness::kReadyForProcess;
    }

    DLOG(INFO) << "[E]GetNodeReadiness()=NodeReadiness::kNotReady";
    return NodeReadiness::kNotReady;
#endif
}

void TestInputStreamHandler::FillInputSet(Timestamp input_timestamp, InputStreamShardSet* input_set) {
    DLOG(INFO) << "[S]FillInputSet()";
    // check
    CHECK(input_timestamp.IsAllowedInStream());
    CHECK(input_set);

    DLOG(INFO) << "input_timestamp=" << input_timestamp;

    for (auto id = input_stream_managers_.BeginId(); id < input_stream_managers_.EndId(); ++id) {
        auto& input_stream_manager = input_stream_managers_.Get(id);
        DLOG(INFO) << "Name()=" << input_stream_manager->Name();
        int num_packets_dropped;
        bool stream_is_done;
        // if there are no packets with current Timestamp, an empty packet with valid Timestamp is returned.
        auto packet = input_stream_manager->PopPacketAtTimestamp(input_timestamp, &num_packets_dropped, &stream_is_done);
        DLOG(INFO) << "PopPacketAtTimestamp(" << input_timestamp << "," << num_packets_dropped << "," << stream_is_done << ")=" << packet;
        // InputStreamManager -> InputStreamShard
        AddPacketToShard(&input_set->Get(id), std::move(packet), stream_is_done);
    }

    DLOG(INFO) << "[E]FillInputSet()";
}

}  // namespace mediapipe
