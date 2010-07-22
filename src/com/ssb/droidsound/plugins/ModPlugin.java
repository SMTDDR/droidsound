package com.ssb.droidsound.plugins;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;


public class ModPlugin implements DroidSoundPlugin {
	
	static {
		System.loadLibrary("modplug");
	}

	public ModPlugin() {
	}

	public boolean canHandle(String name) { return N_canHandle(name); }
	
	@Override
	public Object load(byte [] module, int size) {
		return N_load(module, size);
	}
	@Override
	public void unload(Object song) { N_unload((Long)song); }
	
	// Expects Stereo, 44.1Khz, signed, big-endian shorts
	@Override
	public int getSoundData(Object song, short [] dest, int size) { return N_getSoundData((Long)song, dest, size); }	
	@Override
	public boolean seekTo(Object song, int seconds) { return N_seekTo((Long)song, seconds); }
	@Override
	public boolean setTune(Object song, int tune) { return false; }
	@Override
	public String getStringInfo(Object song, int what) { return N_getStringInfo((Long)song, what); }
	@Override
	public int getIntInfo(Object song, int what) { return N_getIntInfo((Long)song, what); }

	@Override
	public Object loadInfo(File file) throws IOException {
		int l = (int)file.length();
		byte [] songBuffer = new byte [l];
		FileInputStream fs = new FileInputStream(file);
		fs.read(songBuffer);
		long song = N_load(songBuffer, l);
		if(song == 0)
			return null;
		else
			return song;
	}

	@Override
	public Object load(File file) throws IOException {
		int l = (int)file.length();
		byte [] songBuffer = new byte [l];
		FileInputStream fs = new FileInputStream(file);
		fs.read(songBuffer);
		long rc =  N_load(songBuffer, l);
		if(rc == 0)
			return null;
		else
			return rc;
	}

	
	// --- Native functions
	
	native public boolean N_canHandle(String name);
	native public long N_load(byte [] module, int size);
	native public long N_loadInfo(byte [] module, int size);
	native public void N_unload(long song);
	
	// Expects Stereo, 44.1Khz, signed, big-endian shorts
	native public int N_getSoundData(long song, short [] dest, int size);	
	native public boolean N_seekTo(long song, int seconds);
	native public boolean N_setTune(long song, int tune);
	native public String N_getStringInfo(long song, int what);
	native public int N_getIntInfo(long song, int what);

}
