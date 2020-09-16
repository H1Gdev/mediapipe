package com.google.mediapipe.apps.test;

import android.os.Bundle;
import android.util.Log;

import com.google.mediapipe.framework.AndroidPacketCreator;
import com.google.mediapipe.framework.Packet;
import com.google.mediapipe.framework.PacketGetter;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class MainActivity extends com.google.mediapipe.apps.basic.MainActivity {
  private static final String TAG = "MainActivity";

  private static final String OUTPUT_VALUE_STREAM_NAME = "output_value";

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
  }
}
