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

::mediapipe::Status TestSidePacket() {
  CalculatorGraphConfig config = ParseTextProtoOrDie<CalculatorGraphConfig>(R"(
    input_stream: "in"
    output_stream: "out"
    node {
      calculator: "TestCalculator"
      input_stream: "in"
      output_stream: "out"
      input_side_packet: "PACKET:in"
      output_side_packet: "PACKET:out"
    }
  )");

  CalculatorGraph graph;
  MP_RETURN_IF_ERROR(graph.Initialize(config));
  ASSIGN_OR_RETURN(OutputStreamPoller poller, graph.AddOutputStreamPoller("out"));
  MP_RETURN_IF_ERROR(graph.StartRun({
    { "in", MakePacket<std::string>("Input Side Packet...") }
  }));

  for (auto i = 0; i < 10; ++i) {
    auto packet = MakePacket<std::string>("Hello World!").At(Timestamp(i));
    MP_RETURN_IF_ERROR(graph.AddPacketToInputStream("in", packet));
  }

  MP_RETURN_IF_ERROR(graph.CloseInputStream("in"));

  mediapipe::Packet packet;
  while (poller.Next(&packet)) {
    if (packet.ValidateAsType<std::string>().ok())
      LOG(INFO) << packet.Get<std::string>();
  }

  MP_RETURN_IF_ERROR(graph.WaitUntilDone());

  // Get the output side packet by name after the graph is done.
  ASSIGN_OR_RETURN(auto status_or_packet, graph.GetOutputSidePacket("out"));
  if (status_or_packet.ValidateAsType<std::string>().ok())
    LOG(INFO) << "[OUT]Side Packet: " << status_or_packet.Get<std::string>();

  return ::mediapipe::OkStatus();
}

::mediapipe::Status TestThreading() {
#if 0
  // use num_threads.
  CalculatorGraphConfig config = ParseTextProtoOrDie<CalculatorGraphConfig>(R"(
    num_threads: 3

    input_stream: "in"
    output_stream: "out"
    output_stream: "out1"
    node {
      calculator: "TestCalculator"
      input_stream: "in"
      output_stream: "out"
    }
    node {
      calculator: "TestCalculator"
      input_stream: "in"
      output_stream: "out1"
    }
  )");

#if 0
  // Setting in code.
  config.set_num_threads(5);
#endif
#elif 0
  // use executor for default.
  CalculatorGraphConfig config = ParseTextProtoOrDie<CalculatorGraphConfig>(R"(
    # https://github.com/google/mediapipe/blob/master/mediapipe/framework/calculator.proto#L35
    executor {
      # https://github.com/google/mediapipe/blob/master/mediapipe/framework/executor.h
      # https://github.com/google/mediapipe/blob/master/mediapipe/framework/thread_pool_executor.h
      type: "ThreadPoolExecutor"
      options {
        # https://github.com/google/mediapipe/blob/master/mediapipe/framework/thread_pool_executor.proto
        [mediapipe.ThreadPoolExecutorOptions.ext] {
          num_threads: 2
        }
      }
      # "ApplicationThreadExecutor" not work...
      #type: "ApplicationThreadExecutor"
    }

    input_stream: "in"
    output_stream: "out"
    output_stream: "out1"
    node {
      calculator: "TestCalculator"
      input_stream: "in"
      output_stream: "out"
    }
    node {
      calculator: "TestCalculator"
      input_stream: "in"
      output_stream: "out1"
    }
  )");

#if 0
  // Setting in code.
  auto executor = config.add_executor();
  executor->set_type("ThreadPoolExecutor");
  auto options = executor->mutable_options();
  options->MutableExtension(ThreadPoolExecutorOptions::ext)->set_num_threads(2);
#endif
#elif 1
  // use executor(s).
  CalculatorGraphConfig config = ParseTextProtoOrDie<CalculatorGraphConfig>(R"(
    executor {
      # reserved: "default", "gpu"
      name: "test0"
      type: "ThreadPoolExecutor"
      options {
        [mediapipe.ThreadPoolExecutorOptions.ext] {
          num_threads: 2
          require_processor_performance: LOW
        }
      }
    }
    executor {
      name: "test1"
      type: "ThreadPoolExecutor"
      options {
        [mediapipe.ThreadPoolExecutorOptions.ext] {
          num_threads: 2
        }
      }
    }

    input_stream: "in"
    output_stream: "out"
    output_stream: "out1"
    node {
      calculator: "TestCalculator"
      input_stream: "in"
      output_stream: "out"
      executor: "test0"
    }
    node {
      calculator: "TestCalculator"
      input_stream: "in"
      output_stream: "out1"
      executor: "test1"
    }
  )");
#endif

  LOG(INFO) << "[Default]num_threads=" << config.num_threads();

  LOG(INFO) << "[Executor]" << config.executor().size();
  for (const auto& executor : config.executor()) {
    LOG(INFO) << "  " << executor.name() << "(" << executor.type() << ")";
  }

  CalculatorGraph graph;
  MP_RETURN_IF_ERROR(graph.Initialize(config));
  ASSIGN_OR_RETURN(OutputStreamPoller poller, graph.AddOutputStreamPoller("out"));
  MP_RETURN_IF_ERROR(graph.StartRun({}));

  for (auto i = 0; i < 4; ++i) {
    auto packet = MakePacket<std::string>("Threading.").At(Timestamp(i));
    MP_RETURN_IF_ERROR(graph.AddPacketToInputStream("in", packet));
  }

  MP_RETURN_IF_ERROR(graph.CloseInputStream("in"));

  mediapipe::Packet packet;
  while (poller.Next(&packet)) {
    if (packet.ValidateAsType<std::string>().ok())
      LOG(INFO) << packet.Get<std::string>();
  }
  return graph.WaitUntilDone();
}

::mediapipe::Status TestStreamHandler() {
  CalculatorGraphConfig config = ParseTextProtoOrDie<CalculatorGraphConfig>(R"(
    # graph
    input_stream_handler {
      input_stream_handler: "TestInputStreamHandler"
    }
    output_stream_handler {
      output_stream_handler: "TestOutputStreamHandler"
    }

    input_stream: "in"
    output_stream: "out"
    output_stream: "out1"
    node {
      calculator: "TestCalculator"
      input_stream: "in"
      output_stream: "out"

      # node
      input_stream_handler {
        input_stream_handler: "TestInputStreamHandler"
        options: {
          [mediapipe.TestInputStreamHandlerOptions.ext]: {
            name: 'TestCalculator'
          }
        }
      }
      output_stream_handler {
        output_stream_handler: "TestOutputStreamHandler"
        options: {
          [mediapipe.TestOutputStreamHandlerOptions.ext]: {
            name: 'TestCalculator'
          }
        }
      }
    }
    node {
      calculator: "TestCalculator"
      input_stream: "in"
      output_stream: "out1"
    }
  )");

  CalculatorGraph graph;
  MP_RETURN_IF_ERROR(graph.Initialize(config));
  ASSIGN_OR_RETURN(OutputStreamPoller poller, graph.AddOutputStreamPoller("out"));
  MP_RETURN_IF_ERROR(graph.StartRun({}));

  for (auto i = 0; i < 4; ++i) {
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

  LOG(INFO) << "[START]Test Hello World";
  CHECK(mediapipe::TestHelloWorld().ok());
  LOG(INFO) << "[END]Test Hello World";

  LOG(INFO) << "[START]Test Side Packet";
  CHECK(mediapipe::TestSidePacket().ok());
  LOG(INFO) << "[END]Test Side Packet";

  LOG(INFO) << "[START]Test Threading";
  CHECK(mediapipe::TestThreading().ok());
  LOG(INFO) << "[END]Test Threading";

  LOG(INFO) << "[START]Test Stream Handler";
  CHECK(mediapipe::TestStreamHandler().ok());
  LOG(INFO) << "[END]Test Stream Handler";
  return 0;
}
