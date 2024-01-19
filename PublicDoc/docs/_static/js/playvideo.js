// Plays the video with the given ID starting at the given time in seconds.
// This returns a function to pass as an event listener so that the current
// values of video and startTime are curried (instead of using the local value
// after the last iteration of the loop in AddButtonListeners().
function PlayVideoCB(video, startTime) {
    return function() {
        video.play();
        video.pause();
        video.currentTime = startTime;
        video.play();
    }
}

// Find all buttons with class "video-button" and adds a click listener for
// them to play the video with the time set to their "startTime" attribute.
function AddButtonListeners(document) {
    var buttons = document.getElementsByClassName("video-button");
    for (let button of buttons) {
        var videoId   = button.dataset.id;
        var startTime = button.dataset.startTime;
        if (videoId && startTime) {
            var video = document.getElementById(videoId);
            if (video) {
                button.addEventListener('click', PlayVideoCB(video, startTime));
            }
            else {
                console.log("*** No video with id:", videoId);
            }
        }
    }
}

// This has to be done when the document is complete. If this is done on
// DOMContentLoaded, the buttons seem to get replaced or the listeners are
// removed for some reason.
document.addEventListener('readystatechange', function (event) {
    event.preventDefault();
    if (document.readyState == "complete")
        AddButtonListeners(document);
});
