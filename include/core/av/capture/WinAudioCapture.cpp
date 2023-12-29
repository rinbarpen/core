#include "WinAudioCapture.h"

#include <core/util/logger/Logger.h>
LY_NAMESPACE_BEGIN

static auto g_av_logger = GET_LOGGER("av");

void WinAudioCapture::WinAudioCapture() 
{
	auto pCapture = dynamic_cast<WASAPICapture*>(capture_);
	try {
		pCapture->init();
	} catch (std::runtime_error &e) {
	  LOG_ERROR() << e.what();
		return;
	}
  
	WAVEFORMATEX *audioFmt = pCapture->getAudioFormat();
	channels_ = audioFmt->nChannels;
	sample_rate_ = audioFmt->nSamplesPerSec;
	bits_per_sample_ = audioFmt->wBitsPerSample;
	player_->init();

	audio_buffer_.reset(new AudioBuffer(audio_buffer_->capacity()));

	startCapture();

	if (started_)
	  initialized_ = true;
}
void WinAudioCapture::~WinAudioCapture() 
{
  this->stopCapture();
}

void WinAudioCapture::startCapture()
{
	auto pCapture = dynamic_cast<WASAPICapture*>(capture_);
	pCapture->setCallback([this](const WAVEFORMATEX *mixFormat, uint8_t *data, uint32_t samples) {
	  channels_ = mixFormat->nChannels;
	  sample_rate_ = mixFormat->nSamplesPerSec;
	  bits_per_sample_ = mixFormat->wBitsPerSample;
	  audio_buffer_->write(data, mixFormat->nBlockAlign * samples);
	});

	audio_buffer_->clear();
	try {
		pCapture->start();
	} catch (std::runtime_error &e) {
	  ILOG_ERROR(g_av_logger) << e.what();
	  return;
	}

	auto pPlayer = dynamic_cast<WASAPIPlayer*>(player_);
	pPlayer->setCallback([this](const WAVEFORMATEX *mixFormat, uint8_t *data, uint32_t samples) {
		memset(data, 0, mixFormat->nBlockAlign * samples);
	});
	try {
		pPlayer->start();
	} catch(std::runtime_error &e) {
		LOG_ERROR() << e.what();
		return;
	}
	
  started_ = true;
}
void WinAudioCapture::stopCapture()
{
  started_ = false;
  player_->stop();
  capture_->stop();
}
LY_NAMESPACE_END
