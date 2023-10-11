# Ascii Video Player
`ascii-video-player` is simple console program that can play videos in the console as **ASCII** characters

**Warning: Only works on windows**

## How to use
Build the project, run the command
`build\<build type>\asciiVP <path_to_video> [sample_size]`

For exmaple, `build\Release\asciiVP docs\example.mp4`
<video src="docs/default.mp4" muted autoplay title="example"></video>
#### Optional parameters
`sample_size` : control how much characters are used, more characters can result in more detail but introduce more noise to the image (default: 14)
<video src="docs/diff_size.mp4" muted autoplay title="sample_size_showcase"></video>
## How to build
1. Clone the repository
2. Download [ffmpeg](https://ffmpeg.org/download.html#build-windows) and extract then add it to `PATH` (note: download the `share` version)
3. Build by running `build.bat`

## Todos
- Improve performance, for larger video it have a hard time keeping up
- The ability to play audio
- Color support (maybe, windows console only support **16 colors**)

