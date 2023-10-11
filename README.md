# Ascii Video Player
`ascii-video-player` is simple console program that can play videos in the console as **ASCII** characters

**Warning: Only works on windows**

## How to use
Build the project, run the command
`build\<build type>\asciiVP <path_to_video> [sample_size]`

For exmaple, `build\Release\asciiVP docs\example.mp4`
<video src="https://github.com/YarNix/ascii-video-player/assets/123532664/6456c972-7c84-4cf5-894d-7c1c55cfb18c" muted autoplay title="example"></video>
#### Optional parameters
`sample_size` : control how much characters are used, more characters can result in more detail but introduce more noise to the image (default: 14)
<video src="https://github.com/YarNix/ascii-video-player/assets/123532664/8ac39550-bd08-4091-8ee3-3b408fb40966" muted autoplay title="sample_size_showcase"></video>
## How to build
1. Clone the repository
2. Download [ffmpeg](https://ffmpeg.org/download.html#build-windows) and extract then add it to `PATH` (note: download the `share` version)
3. Build by running `build.bat`

## Todos
- Improve performance, for larger video it have a hard time keeping up
- The ability to play audio
- Color support (maybe, windows console only support **16 colors**)

