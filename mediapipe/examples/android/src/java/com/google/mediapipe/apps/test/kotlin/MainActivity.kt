package com.google.mediapipe.apps.test.kotlin

import android.os.Bundle
import android.util.Log
import com.google.mediapipe.apps.basic.MainActivity
import com.google.mediapipe.framework.Packet
import com.google.mediapipe.framework.PacketGetter
import java.util.Arrays
import java.util.HashMap

class MainActivity : MainActivity() {

  companion object {
    private const val TAG = "MainActivity"
    private const val OUTPUT_VALUE_STREAM_NAME = "output_value"
  }

  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)

    // Set input side packet.
    val inputSidePackets: MutableMap<String, Packet> = HashMap()
    val packetCreator = processor.packetCreator
    inputSidePackets["user_value_0"] = packetCreator.createFloat32(-4.4f)
    // To subgraph.
    inputSidePackets["testsubgraph__user_value_0"] = packetCreator.createString("Input Side Packet...")
    processor.setInputSidePackets(inputSidePackets)

    // $ adb shell setprop log.tag.MainActivity VERBOSE
    if (Log.isLoggable(TAG, Log.VERBOSE)) {
      processor.addPacketCallback(OUTPUT_VALUE_STREAM_NAME) { packet: Packet ->
        val outputValue = PacketGetter.getFloat32Vector(packet)
        Log.v(TAG, "output_value is " + Arrays.toString(outputValue) + " (" + packet.timestamp + ")")
      }
    }
  }
}
