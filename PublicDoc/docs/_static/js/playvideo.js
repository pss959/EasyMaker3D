// Plays the video with the given ID starting at the given time in seconds.
function PlayVideo(video, seconds){
    video.play();
    video.pause();
    video.currentTime = seconds;
    video.play();
}

// Find all buttons with class "video-button" and have them play the video with
// the time set to their "time" attribute.
document.addEventListener('DOMContentLoaded', function (event) {
    event.preventDefault();
    buttons = document.getElementsByClassName("video-button");
    for (var i = 0; i < buttons.length; ++i) {
        var id      = buttons[i].dataset.id;
        var seconds = buttons[i].dataset.seconds;
        buttons[i].setAttribute("onclick",
                                `javascript:PlayVideo(${id}, ${seconds})`);
    }
});
