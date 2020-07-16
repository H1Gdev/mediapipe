#include "mediapipe/framework/calculator_graph.h"
#include "mediapipe/framework/deps/file_path.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"

namespace mediapipe {

::mediapipe::Status ExecuteSNPE() {
  CalculatorGraphConfig config = ParseTextProtoOrDie<CalculatorGraphConfig>(R"(
    input_stream: "in"
    output_stream: "out"
    node {
      calculator: "SnpeInferenceCalculator"
      input_stream: "in"
      output_stream: "out"
      node_options: {
        [type.googleapis.com/mediapipe.SnpeInferenceCalculatorOptions] {
          model_file_path: "mediapipe/models/model.dlc"
        }
      }
    }
  )");

  CalculatorGraph graph;
  MP_RETURN_IF_ERROR(graph.Initialize(config));
  ASSIGN_OR_RETURN(OutputStreamPoller poller, graph.AddOutputStreamPoller("out"));
  MP_RETURN_IF_ERROR(graph.StartRun({}));

  {
    // make Packet.
    std::string contents;
    MP_RETURN_IF_ERROR(file::GetContents(
      file::JoinPath("./", "mediapipe/examples/desktop/snpe/input.float32.raw"),
      &contents));
    // to float.
    std::vector<float> data(contents.size() / sizeof(float));
    std::memcpy(data.data(), contents.data(), contents.size());

    auto packet = MakePacket<std::vector<float>>(data).At(Timestamp(0));
    MP_RETURN_IF_ERROR(graph.AddPacketToInputStream("in", packet));
  }

  // Close the input stream "in".
  MP_RETURN_IF_ERROR(graph.CloseInputStream("in"));

  mediapipe::Packet packet;
  while (poller.Next(&packet)) {
    if (packet.ValidateAsType<std::vector<float>>().ok()) {
      const auto& data = packet.Get<std::vector<float>>();
      // to char.
      std::vector<char> c(data.size() * sizeof(float));
      std::memcpy(c.data(), data.data(), c.size());
      // to string.
      std::string contents(c.begin(), c.end());
      MP_RETURN_IF_ERROR(file::SetContents(
        file::JoinPath("./", "mediapipe/examples/desktop/snpe/output.float32.raw"),
        contents));
    }
  }

  return graph.WaitUntilDone();
}
}  // namespace mediapipe

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);

  CHECK(mediapipe::ExecuteSNPE().ok());
  return 0;
}
