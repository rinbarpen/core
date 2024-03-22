#include <core/config/config.h>
#include <core/multimedia/player/Player.h>

LY_NAMESPACE_BEGIN
static auto g_player_logger = GET_LOGGER3("multimedia.player");
static auto g_ffmpeg_open_debug = LY_CONFIG_GET(multimedia.ffmpeg.open_debug);

Player::Player() {
  ffmpeg::reg_ffmpeg(ffmpeg::RegFlag::ALL);
}
Player::~Player() {
  this->close();
}

bool Player::prepare(PlayerConfig config) {
  if (config.video.format == AV_PIX_FMT_NONE) {
    return false;
  }
  if (config.video.width <= 0 && config.video.height <= 0) {
    config.video.width = 1080;
    config.video.height = 720;
  }
  if (config.video.frame_rate <= 0) {
    config.video.frame_rate = 25;
  }
  if (config.video.bit_rate <= 0) {
    return false;
  }

  if (config.audio.format == AV_SAMPLE_FMT_NONE) {
    return false;
  }
  if (config.audio.bit_rate <= 0) {
    return false;
  }

  LY_ASSERT(config.common.speed <= 0.0f);
  config_ = config;
  initialized_ = true;
  return true;
}
bool Player::close() {
  if (initialized_) {
    if (status_ == PlayerStatus::PLAYING
     || status_ == PlayerStatus::PAUSE) {
      teardown();
    }

    video_stream_index_ = audio_stream_index_ = -1;
    vcodec_id_ = acodec_id_ = AV_CODEC_ID_NONE;
  }
  return true;
}

bool Player::openUrl(std::string_view url) {
  if (status_ == PlayerStatus::PLAYING
   || status_ == PlayerStatus::PAUSE) {
    this->close();
  }

  AVFormatContext *format_context = avformat_alloc_context();
  int r{0};
  r = avformat_open_input(&format_context, url.data(), nullptr, nullptr);
  if (r < 0) {
    // error url
    ILOG_ERROR_FMT(g_player_logger, "Error opening the url: {}", url);
    return false;
  }
  r = avformat_find_stream_info(format_context, nullptr);
  if (r < 0) {
    ILOG_ERROR_FMT(g_player_logger, "Could not find stream information");
    return false;
  }

  if (g_ffmpeg_open_debug) av_dump_format(format_context, 0, url.data(), 0);

  video_stream_index_ = -1;
  audio_stream_index_ = -1;
  for (int i = 0; i < format_context->nb_streams; i++) {
    if (video_stream_index_ == -1
        && AVMEDIA_TYPE_VIDEO
             == format_context->streams[i]->codecpar->codec_type) {
      video_stream_index_ = i;
    }
    else if (audio_stream_index_ == -1
             && AVMEDIA_TYPE_AUDIO
                  == format_context->streams[i]->codecpar->codec_type) {
      audio_stream_index_ = i;
    }
  }

  AVCodecParameters *video_codec_parameters{nullptr};
  AVCodecParameters *audio_codec_parameters{nullptr};
  const AVCodec *vcodec{nullptr}, *acodec{nullptr};
  if (config_.common.video_on) {
    if (video_stream_index_ == -1) {
      ILOG_WARN_FMT(
        g_player_logger, "Could not find video stream in the input file");
      config_.common.video_on = false;
    }
    else {
      video_codec_parameters =
        format_context->streams[video_stream_index_]->codecpar;
      vcodec = avcodec_find_decoder(video_codec_parameters->codec_id);
      if (nullptr == vcodec) {
        ILOG_ERROR_FMT(
          g_player_logger, "Could not find decoder for the video stream");
        return false;
      }
      AVCodecContext *codec_context = avcodec_alloc_context3(vcodec);
      if (nullptr == codec_context) {
        ILOG_ERROR(g_player_logger) << "Could not allocate video codec context";
        return false;
      }
      if (avcodec_parameters_to_context(codec_context, video_codec_parameters)
          < 0) {
        ILOG_ERROR(g_player_logger)
          << "Could not copy codec parameters to codec context";
        return false;
      }
      if (avcodec_open2(codec_context, vcodec, 0) < 0) {
        ILOG_ERROR(g_player_logger) << "Could not open video codec";
        return false;
      }
    }
  }
  else {
    ILOG_TRACE(g_player_logger) << "Disable video";
  }

  if (config_.common.audio_on) {
    if (audio_stream_index_ == -1) {
      ILOG_WARN_FMT(
        g_player_logger, "Could not find audio stream in the input file");
      config_.common.audio_on = false;
    }
    else {
      audio_codec_parameters =
        format_context->streams[audio_stream_index_]->codecpar;
      acodec = avcodec_find_decoder(audio_codec_parameters->codec_id);
      if (nullptr == acodec) {
        ILOG_ERROR_FMT(
          g_player_logger, "Could not find decoder for the audio stream");
        return false;
      }
      AVCodecContext *codec_context = avcodec_alloc_context3(vcodec);
      if (nullptr == codec_context) {
        ILOG_ERROR(g_player_logger) << "Could not allocate video codec context";
        return false;
      }
      if (avcodec_parameters_to_context(codec_context, audio_codec_parameters)
          < 0) {
        ILOG_ERROR(g_player_logger)
          << "Could not copy codec parameters to codec context";
        return false;
      }
      if (avcodec_open2(codec_context, acodec, 0) < 0) {
        ILOG_ERROR(g_player_logger) << "Could not open audio codec";
        return false;
      }
    }
  }
  else {
    ILOG_TRACE(g_player_logger) << "Disable audio";
  }

  if (config_.common.video_on) {
    video_decoder_thread_.dispatch([this](){
      this->onDecodeVideo();
    });
  }
  if (config_.common.audio_on) {{
    audio_decoder_thread_.dispatch([this](){
      this->onDecodeAudio();
    });
  }}

  vcodec_id_ = video_codec_parameters ? video_codec_parameters->codec_id : AV_CODEC_ID_NONE;
  acodec_id_ = audio_codec_parameters ? audio_codec_parameters->codec_id : AV_CODEC_ID_NONE;

  this->play();
  return true;
}

void Player::play() {
  status_ = PlayerStatus::PLAYING;
}
void Player::pause() {
  status_ = PlayerStatus::PAUSE;
}
void Player::teardown() {
  status_ = PlayerStatus::NO_SOURCE;
}

void Player::onDecodeVideo() {

}
void Player::onDecodeAudio() {

}

LY_NAMESPACE_END
