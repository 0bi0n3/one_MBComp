# One_MBCompAudioProcessor - Multiband Compressor Plugin

This project provides a multiband compressor plugin for Audio Programming 2, assignment 2. The plugin is built upon the JUCE framework and offers low, mid, and high-band compressors, each with its set of controls.

## Features:

- **3-Band compression**: Allows low, mid, and high-frequency compression.
- **Gain Control**: Individual gain controls for input and output stages.
- **Crossover Filters**: Customize the crossover frequencies between bands.
- **Solo and Mute Options**: Solo or mute each compression band.
- **Various Compressor Parameters**: Customize attack, release, threshold, and ratio for each band.

## Compiling with JUCE v6:

1. Make sure you have installed JUCE v6.
2. Open the `.jucer` file associated with this project in the Projucer.
3. Configure your settings and IDE in Projucer if necessary.
4. Save and Open in IDE using the appropriate option in Projucer.
5. Compile and build the project from your IDE.

**Note for building**
Occasionally the project has incorrectly included some of the source files within the source directory. Ensure that all `.cpp` and `.h` files are present in the source file within the project. If not present, drag the files from the source folder into the project folder.

## Basic Controls:

**Compressor Parameters:**

- `Attack_LB, Attack_MB, Attack_HB`: Attack time for Low, Mid, and High bands, respectively.
- `Release_LB, Release_MB, Release_HB`: Release time for the three bands.
- `Threshold_LB, Threshold_MB, Threshold_HB`: Threshold level settings for the bands.
- `Ratio_LB, Ratio_MB, Ratio_HB`: Compression ratios for the bands.
  
**Bypass, Mute, and Solo Options:**

- `Bypass_LB, Bypass_MB, Bypass_HB`: Bypass individual bands.
- `Mute_LB, Mute_MB, Mute_HB`: Mute individual bands.
- `Solo_LB, Solo_MB, Solo_HB`: Solo individual bands.

**Other Settings:**

- `Gain_Input`: Adjust the input gain.
- `Gain_Output`: Adjust the output gain.
- `Low_Mid_XO_Frequency`: Set the crossover frequency between low and mid bands.
- `Mid_High_XO_Frequency`: Set the crossover frequency between mid and high bands.

By navigating the plugin's GUI, you can adjust these parameters according to your requirements and listen to the real-time changes.
