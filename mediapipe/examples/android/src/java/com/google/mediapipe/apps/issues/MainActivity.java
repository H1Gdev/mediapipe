package com.google.mediapipe.apps.issues;

import android.os.Bundle;

import com.google.mediapipe.framework.AndroidPacketCreator;
import com.google.mediapipe.framework.Packet;

import java.util.HashMap;
import java.util.Map;

public class MainActivity extends com.google.mediapipe.apps.basic.MainActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Map<String, Packet> inputSidePackets = new HashMap<>();
        AndroidPacketCreator packetCreator = processor.getPacketCreator();
        inputSidePackets.put("file_path0", packetCreator.createString("labelmap0.txt"));
        inputSidePackets.put("file_path1", packetCreator.createString("labelmap1.txt"));
        processor.setInputSidePackets(inputSidePackets);
    }
}
