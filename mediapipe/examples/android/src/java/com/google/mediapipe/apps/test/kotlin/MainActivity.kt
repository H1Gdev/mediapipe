package com.google.mediapipe.apps.test.kotlin

import android.os.Bundle
import android.util.Log
import com.google.mediapipe.apps.basic.MainActivity
import com.google.mediapipe.formats.proto.DetectionProto
import com.google.mediapipe.framework.Packet
import com.google.mediapipe.framework.PacketGetter
import com.google.mediapipe.framework.ProtoUtil
import com.google.protobuf.InvalidProtocolBufferException
import java.util.Arrays
import java.util.HashMap

class MainActivity : MainActivity() {

  companion object {
    init {
      // https://github.com/google/mediapipe/issues/1013
      ProtoUtil.registerTypeName<DetectionProto.Detection>(DetectionProto.Detection::class.java, "mediapipe.Detection")
    }
    private const val TAG = "MainActivity"
    private const val OUTPUT_VALUE_STREAM_NAME = "output_value"
    private const val DETECTION_STREAM_NAME = "detection"
    private const val DETECTIONS_STREAM_NAME = "detections"
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

    // for Detection
    processor.addPacketCallback(
            DETECTION_STREAM_NAME
    ) { packet: Packet? ->
      // not called Callback if Packet is empty.
      try {
        val detection: DetectionProto.Detection = PacketGetter.getProto(packet, DetectionProto.Detection::class.java)
        Log.d(TAG, "Detection is $detection")
      } catch (e: InvalidProtocolBufferException) {
        Log.e(TAG, "PacketGetter.getProto()", e)
      }
    }
    processor.addPacketCallback(
            DETECTIONS_STREAM_NAME
    ) { packet: Packet? ->
      val detections: List<DetectionProto.Detection> = PacketGetter.getProtoVector(packet, DetectionProto.Detection.parser())
      Log.d(TAG, "Detections is $detections")
    }
  }
}
