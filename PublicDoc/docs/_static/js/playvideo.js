// Plays the video with the given ID starting at the given time in seconds.
function PlayVideo(video, startTime) {
    video.play();
    video.pause();
    video.currentTime = startTime;
    video.play();
}

// Find all buttons with class "video-button" and adds a click listener for
// them to play the video with the time set to their "startTime" attribute.
function AddButtonListeners(document) {
    buttons = document.getElementsByClassName("video-button");
    for (var i = 0; i < buttons.length; ++i) {
        var videoId   = buttons[i].dataset.id;
        var startTime = buttons[i].dataset.startTime;
        if (videoId && startTime) {
            var video = document.getElementById(videoId);
            if (video) {
                buttons[i].addEventListener('click', e => {
                    PlayVideo(video, startTime);
                });
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
