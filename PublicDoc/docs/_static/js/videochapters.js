// -----------------------------------------------------------------------------
// Adds UI to display chapters for videos and to allow the chapters to be
// selected to play them.
//
// NOTE: This assumes there is at most one video per HTML page.
// -----------------------------------------------------------------------------

var haveMetadata = false;
var video;
var chapterBar;

// This is set to onloadedmetadata in the HTML file so that it must be invoked
// even before DOMContentLoaded is called.
function metadataLoaded() {
    haveMetadata = true;
    if (video) {
        addChapters();
    }
}

// Called when video is clicked.
function playOrPause() {
    if (video.paused) { video.play(); } else { video.pause(); }
}

// Changes the color in the chapter bar to show the current time.
function updateTime() {
    var t = video.currentTime / video.duration * 100;
    chapterbar.style.background =
        "linear-gradient(to right, #500 " + t + "%, #000 " + t + "%)";
}

function addChapters() {
    // Don't do this more than once.
    if (chapterbar.childElementCount > 0)
        return;

    // Cannot do this properly until the video duration is known and
    // the tracks have been loaded.
    var cues = video.textTracks[0].cues;
    var margin = 8;
    if (video.duration > 0 && cues && cues.length > 0) {
        for (var i=0; i<cues.length; i++) {
            var cue = cues[i];
            var span = document.createElement("span");
            var duration = cue.endTime - cue.startTime;
            span.innerHTML = cue.text;
            span.setAttribute("class", "chapter-span");
            span.setAttribute("data-start-time", cue.startTime);
            span.style.width = (100 * duration / video.duration) + "%";
            span.addEventListener("click", seekToChapterStart);
            chapterbar.appendChild(span);
        }
    }
}

function seekToChapterStart() {
    var video = document.querySelector("video");
    video.currentTime = this.getAttribute("data-start-time");
    if (video.paused) { video.play(); }
}

document.addEventListener("DOMContentLoaded", function (event) {
    event.preventDefault();
    video = document.querySelector("video");
    var track = document.querySelector("track");
    chapterBar = document.querySelector("#chapterbar");
    if (video && track && chapterBar) {
        video.addEventListener("click",       playOrPause);
        video.addEventListener("seeked",      updateTime);
        video.addEventListener("timeupdate",  updateTime);
        track.addEventListener("load",        addChapters);
        if (haveMetadata) {
            addChapters();
        }
    }
});

