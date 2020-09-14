#include "mediapipe/framework/output_stream_handler.h"

#include "mediapipe/framework/stream_handler/test/test_output_stream_handler.pb.h"

namespace mediapipe {

// Output Stream Handler
class TestOutputStreamHandler : public OutputStreamHandler {
public:
    TestOutputStreamHandler() = delete;
    TestOutputStreamHandler(
        std::shared_ptr<tool::TagMap> tag_map,
        CalculatorContextManager* calculator_context_manager,
        const MediaPipeOptions& options,
        bool calculator_run_in_parallel);

protected:
    void PropagationLoop() ABSL_EXCLUSIVE_LOCKS_REQUIRED(timestamp_mutex_) override;
};

REGISTER_OUTPUT_STREAM_HANDLER(TestOutputStreamHandler);

TestOutputStreamHandler::TestOutputStreamHandler(
    std::shared_ptr<tool::TagMap> tag_map,
    CalculatorContextManager* calculator_context_manager,
    const MediaPipeOptions& options,
    bool calculator_run_in_parallel)
    : OutputStreamHandler(std::move(tag_map), calculator_context_manager, options, calculator_run_in_parallel) {
    DLOG(INFO) << "[S]TestOutputStreamHandler()";
    // options
    if (options.HasExtension(TestOutputStreamHandlerOptions::ext)) {
        const auto& stream_handler_options = options.GetExtension(TestOutputStreamHandlerOptions::ext);
        if (stream_handler_options.has_name())
            DLOG(INFO) << "name is " << stream_handler_options.name();
    }
    DLOG(INFO) << "[E]TestOutputStreamHandler()";
}

void TestOutputStreamHandler::PropagationLoop() {
    DLOG(INFO) << "[S]PropagationLoop()";
    DLOG(INFO) << "[E]PropagationLoop()";
}

}  // namespace mediapipe
