package com.google.mediapipe.apps.test.java;

import android.os.Bundle;
import android.util.Log;

import com.google.mediapipe.formats.proto.DetectionProto;
import com.google.mediapipe.framework.AndroidPacketCreator;
import com.google.mediapipe.framework.Packet;
import com.google.mediapipe.framework.PacketGetter;
import com.google.mediapipe.framework.ProtoUtil;
import com.google.protobuf.InvalidProtocolBufferException;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class MainActivity extends com.google.mediapipe.apps.basic.MainActivity {
  private static final String TAG = "MainActivity";

  private static final String OUTPUT_VALUE_STREAM_NAME = "output_value";
  private static final String DETECTION_STREAM_NAME = "detection";
  private static final String DETECTIONS_STREAM_NAME = "detections";

  static {
      // https://github.com/google/mediapipe/issues/1013
      ProtoUtil.registerTypeName(DetectionProto.Detection.class, "mediapipe.Detection");
  }

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    // Set input side packet.
    Map<String, Packet> inputSidePackets = new HashMap<>();
    AndroidPacketCreator packetCreator = processor.getPacketCreator();
    inputSidePackets.put("user_value_0", packetCreator.createFloat32(-4.4f));
    // To subgraph.
    inputSidePackets.put("testsubgraph__user_value_0", packetCreator.createString("Input Side Packet..."));
    processor.setInputSidePackets(inputSidePackets);

    // $ adb shell setprop log.tag.MainActivity VERBOSE
    if (Log.isLoggable(TAG, Log.VERBOSE)) {
      processor.addPacketCallback(
              OUTPUT_VALUE_STREAM_NAME,
              (packet) -> {
                float[] outputValue = PacketGetter.getFloat32Vector(packet);
                Log.v(TAG, "output_value is " + Arrays.toString(outputValue) + " (" + packet.getTimestamp() + ")");
              });
    }

    // for Detection
    processor.addPacketCallback(
            DETECTION_STREAM_NAME,
            (packet -> {
                // not called Callback if Packet is empty.
                try {
                    DetectionProto.Detection detection = PacketGetter.getProto(packet, DetectionProto.Detection.class);
                    Log.d(TAG, "Detection is " + detection);
                } catch (InvalidProtocolBufferException e) {
                    Log.e(TAG, "PacketGetter.getProto()", e);
                }
            }));
      processor.addPacketCallback(
              DETECTIONS_STREAM_NAME,
              (packet -> {
                  List<DetectionProto.Detection> detections = PacketGetter.getProtoVector(packet, DetectionProto.Detection.parser());
                  Log.d(TAG, "Detections is " + detections);
              }));
  }
}
