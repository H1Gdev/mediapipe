#include "mediapipe/framework/calculator_graph.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"

namespace mediapipe {

::mediapipe::Status TestHelloWorld() {
  // https://github.com/google/mediapipe/blob/v0.7.5/mediapipe/framework/calculator.proto#L218
  CalculatorGraphConfig config = ParseTextProtoOrDie<CalculatorGraphConfig>(R"(
    input_stream: "in"
    output_stream: "out"
    node {
      calculator: "TestCalculator"
      input_stream: "in"
      output_stream: "out"
      node_options: {
        [type.googleapis.com/mediapipe.TestCalculatorOptions] {
          test_name: "test"
        }
      }
    }
  )");

  CalculatorGraph graph;
  MP_RETURN_IF_ERROR(graph.Initialize(config));
  ASSIGN_OR_RETURN(OutputStreamPoller poller, graph.AddOutputStreamPoller("out"));
  MP_RETURN_IF_ERROR(graph.StartRun({}));

  for (auto i = 0; i < 10; ++i) {
    // make Packet.
    // Packet is chain style.
    auto packet = MakePacket<std::string>("Hello World!").At(Timestamp(i));
    MP_RETURN_IF_ERROR(graph.AddPacketToInputStream("in", packet));
  }

  // Close the input stream "in".
  MP_RETURN_IF_ERROR(graph.CloseInputStream("in"));

  // Get the output packets std::string.
  mediapipe::Packet packet;
  while (poller.Next(&packet)) {
    // Check packet type.
    if (packet.ValidateAsType<std::string>().ok())
      // Output Log.
      LOG(INFO) << packet.Get<std::string>();
  }
  return graph.WaitUntilDone();
}
}  // namespace mediapipe

int main(int argc, char** argv) {
  // Use glog.
  google::InitGoogleLogging(argv[0]);

#if 1
    LOG(INFO) << "test log of INFO level.";
    LOG(WARNING) << "test log of WARNING level.";
    LOG(ERROR) << "test log of ERROR level.";
#if 0
    // Kill process.
    // ref. google::InstallFailureSignalHandler()
    LOG(FATAL) << "test log of FATAL level.";
#endif

    // For Debug.
    DLOG(INFO) << "test debug log of INFO level.";
#endif

  CHECK(mediapipe::TestHelloWorld().ok());
  return 0;
}
