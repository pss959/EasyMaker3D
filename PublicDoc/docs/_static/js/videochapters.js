// -----------------------------------------------------------------------------
// Adds UI to display chapters for videos and to allow the chapters to be
// selected to play them.
//
// NOTE: This assumes there is at most one video per HTML page.
// -----------------------------------------------------------------------------

document.addEventListener('DOMContentLoaded', function (event) {
    event.preventDefault();
    // If there is a video on this page with chapter data, try to add the
    // chapters bar UI after loading the track data.
    var video = document.querySelector('video');
    if (video) {
        var tracks = video.getElementsByTagName('track');
        if (tracks && tracks.length > 0)
            tracks[0].addEventListener('load', addChapters);
    }
});

// Sets up the navigation bar with chapter data.
function addChapters() {
    // Don't do this more than once.
    var chapterBar = document.querySelector("#chapterbar");
    if (chapterbar && chapterbar.childElementCount > 0)
        return;

    // Cannot do this until video metadata and track data have been loaded.
    var video = document.querySelector('video');
    if (! video || video.readyState < 1 || ! video.textTracks.length)
        return;

    var cues = video.textTracks[0].cues;
    for (var i=0; i<cues.length; i++) {
        var cue = cues[i];
        var span = document.createElement('span');
        var duration = cue.endTime - cue.startTime;
        span.innerHTML = cue.text;
        span.setAttribute('class', 'chapter-span');
        span.setAttribute('data-start-time', cue.startTime);
        span.style.width = (100 * duration / video.duration) + '%';
        span.addEventListener('click', playChapter);
        chapterbar.appendChild(span);
    }

    // The above code changes the document, invalidating all element
    // references, so access the video again to set up the listener.
    video = document.querySelector('video');
    video.addEventListener('timeupdate', updateTime);
}

// Changes the colors in the chapter bar to show the current time.
function updateTime() {
    var video      = document.querySelector('video');
    var chapterBar = document.querySelector("#chapterbar");

    // The chapter bar transitions between these colors to show the current
    // time.
    var color0 = getComputedStyle(chapterBar).getPropertyValue("--bg-color0");
    var color1 = getComputedStyle(chapterBar).getPropertyValue("--bg-color1");

    var percent = ' ' + (100 * video.currentTime / video.duration) + '%';
    chapterbar.style.background = 'linear-gradient(to right, ' +
        color0 + percent + ', ' + color1 + percent + ')';
}

function playChapter() {
    var video = document.querySelector('video');
    video.currentTime = this.getAttribute('data-start-time');
    if (video.paused)
        video.play();
}
