// -----------------------------------------------------------------------------
// Adds UI to display chapters for videos and to allow the chapters to be
// selected to play them. Requires the video to have the width set properly.
//
// NOTE: This assumes there is at most one video per HTML page.
// -----------------------------------------------------------------------------

document.addEventListener('DOMContentLoaded', function (event) {
    // Called when video is clicked.
    function playOrPause() {
        var video = document.querySelector("video");
        console.log("XXXX In playOrPause");
        if (video.paused) { video.play(); } else { video.pause(); }
    }

    // Changes the color in the chapter bar to show the current time.
    function updateTime() {
        var video      = document.querySelector("video");
        var chapterbar = document.querySelector("#chapterbar");
        console.log("XXXX UPDATE t =", t, "CB=", chapterbar); // XXXX
        var t = video.currentTime / video.duration * 100;
        chapterbar.style.background =
            "linear-gradient(to right, #500 " + t + "%, #000 " + t + "%)";
    }

    function drawChapters() {
        console.log("XXXX In drawChapters");
        var video      = document.querySelector("video");
        var chapterbar = document.querySelector("#chapterbar");
        // Cannot do this properly until the video duration is known and
        // the tracks have been loaded.
        var cues = video.textTracks[0].cues;
        var margin = 8;
        if (video.duration > 0 && cues && cues.length > 0) {
            chapterbar.style.width = video.width + "px";
            console.log("XXXX CBW=", chapterbar.style.width);
            for (var i=0; i<cues.length; i++) {
                var cue = cues[i];
                var span = document.createElement("span");
                var duration = cue.endTime - cue.startTime;
                span.innerHTML = cue.text;
                span.setAttribute('data-start-time', cue.startTime);
                span.style.width =
                    (duration * video.width / video.duration - margin) + 'px';
                span.addEventListener("click", seekToChapterStart);
                chapterbar.appendChild(span);
            }
        }
    }

    function seekToChapterStart() {
        var video = document.querySelector("video");
        video.currentTime = this.getAttribute('data-start-time');
        if (video.paused) { video.play(); }
    }

    event.preventDefault();
    var video = document.querySelector("video");
    var track = document.querySelector("track");
    if (video && track) {
        // NOTE: The loadeddata event is used because that is the only event
        // that seems to guarantee that the video duration is known (AND only
        // if preload="auto" in the video element. The "durationchange" and
        // "loadedmetadata" events do not seem to guarantee this.
        video.addEventListener('click',       playOrPause);
        video.addEventListener('seeked',      updateTime);
        video.addEventListener('timeupdate',  updateTime);
        video.addEventListener('loadeddata',  drawChapters);
        track.addEventListener('load',        drawChapters);
    }
});

