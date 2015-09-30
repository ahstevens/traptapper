# traptapper
A command-line utility for batch-extracting camera trap data using [Tesseract-OCR](https://github.com/tesseract-ocr)

## Download
The latest traptapper Win32 binary and Tesseract-OCR Win32 DLL can be downloaded [here](https://github.com/ahstevens/traptapper/releases/latest).

**Note**: The Tesseract-OCR v3.02 DLL is **required**, and should be kept in the same directory as the traptapper executable.

## How It Works
traptapper attempts to cut down the amount of time spent on camera trap data entry by using open-source optical character recognition (OCR) software to automatically scan and extract the pertinent information from the camera trap pictures.

### Command Line Usage
```PowerShell
 traptapper <output_file> <camera_trap_profile> <image_list_file>
```

For example:
```PowerShell
 traptapper output_file.csv bushnell_camera.cfg my_images.txt
```
 
### Camera Trap Profiles
Profiles can be created for different models of camera traps, or for different sets of target data for the same camera trap model.

The format of the camera profiles is as follows:
```
 Camera Model Name or Profile Name
 <parameter_name> <left> <top> <width> <height> [size]
 [parameter_name] ...
```
In other words, the first line of the profile is a text string used as a profile identifier, followed by at least one parameter specification.
Parameter specifications require a name and four values that describe the parameter's bounding box in pixel space, with an optional value specifying the parameter's size.

The specifications for describing the parameter(s) are:
- `<parameter_name>`: a name for the parameter. Note that this name can contain **only** alphanumeric characters and the underscore (`_`) character. The name must have **no spaces or commas**.
  - if a parameter name is specified as `date`, `time`, `temperature`, or `temp`, extra data scrubbing functions will be applied to the output value in an attempt to produce cleaner output.
- `<left>`: the pixel coordinate of the left edge of the bounding box (i.e., the x-coordinate of the bounding box's top-left corner)
- `<top>`: the pixel coordinate of the top edge of the bounding box (i.e., the y-coordinate of the bounding box's top-left corner)
- `<width>`: the pixel width of the bounding box
- `<height>`: the pixel height of the bounding box
- `[size]`: An optional value to specify the expected size of the parameter, typically useful for fixed numeric parameters. For example, if a temperature parameter is always given in the format `##.#`, a size value of 4 can be used to help trim any extra cruft picked up by the OCR software.

**Note:** at least one parameter must be specified, but there is no limit to the number of parameters that can be specified in a profile.

Some sample camera profile configuration files can be found [here](https://github.com/ahstevens/traptapper/tree/master/configs).

### Image List File
The images to be processed should be specified in a plain text file with each image's path on its own individual line.

For example:
```
 C:\Users\MyUsername\Pictures\CameraTrap\IMG0001.jpg
 C:\Users\MyUsername\Pictures\CameraTrap\IMG0002.jpg
 C:\Users\MyUsername\Pictures\CameraTrap\IMG0003.jpg
 [...]
```

**Note:** absolute file paths are ideal, but relative file paths can be used as well.

### Output
traptapper outputs its results in comma-separated value (CSV) format, with one result per line.

For example, the output using a camera profile named "Bushnell Model X" which specifies `temperature`, `date`, and `time` parameters (in that order) along with the image list in the above example would be formatted as follows:

```
filepath,cameraType,temperature,date,time
C:\Users\MyUsername\Pictures\CameraTrap\IMG0001.jpg,Bushnell Model X,22.1,11/15/2015,14:32:50
C:\Users\MyUsername\Pictures\CameraTrap\IMG0002.jpg,Bushnell Model X,21.4,11/15/2015,14:35:32
C:\Users\MyUsername\Pictures\CameraTrap\IMG0003.jpg,Bushnell Model X,23.2,11/15/2015,14:36:54
```
