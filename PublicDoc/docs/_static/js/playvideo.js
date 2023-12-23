// Plays the video with the given ID starting at the given time in seconds.
function PlayVideo(video, start_time){
    video.play();
    video.pause();
    video.currentTime = start_time;
    video.play();
}

// Find all buttons with class "video-button" and have them play the video with
// the time set to their "time" attribute.
document.addEventListener('DOMContentLoaded', function (event) {
    event.preventDefault();
    buttons = document.getElementsByClassName("video-button");
    for (var i = 0; i < buttons.length; ++i) {
        var id        = buttons[i].dataset.id;
        var startTime = buttons[i].dataset.startTime;
        if (id && startTime)
            buttons[i].setAttribute(
                "onclick", `javascript:PlayVideo(${id}, ${startTime})`);
    }
});
