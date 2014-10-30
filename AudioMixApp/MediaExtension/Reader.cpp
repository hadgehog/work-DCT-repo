#include "pch.h"
#include "Reader.h"
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mftransform.h>
#include <mferror.h>
#include <wrl.h>
#include <vector>
#include <xaudio2.h>
#include <xaudio2fx.h>
#include <propvarutil.h>
#include "AudioReader.h"
#include "MFAudioReader.h"
#include "MFAudioEvents.h"

using namespace MediaExtension;
using namespace Platform;


Reader::Reader()
{
	this->player = std::shared_ptr<XAudio2Player>(new XAudio2Player());
}

void Reader::Play(IPlayList ^playList)
{
	InitMasterVoice::GetInstance();
	Windows::Storage::Streams::IRandomAccessStream ^stream;
	MFAudioReader *reader = new MFAudioReader();
	this->currentPlayList = playList;

	{
		MFAudioEvents *tmppEvents = new MFAudioEvents();
		tmppEvents->InitEvent(this);
		this->events = std::shared_ptr<AudioEvents>(tmppEvents);
	}	
	
	stream = currentPlayList->GetStream(trackNumber);
	this->xAudio2 = InitMasterVoice::GetInstance().GetXAudio();

	reader->Initialize(stream);
	this->player->Initialize(reader, this->xAudio2, this->events);

	this->playersList.push_back(this->player);
}

void Reader::Rewinding(double setPosition)
{
	this->player->SetPosition(Rational::SEC, setPosition);
}

Windows::Foundation::TimeSpan Reader::Duration::get()
{
	Windows::Foundation::TimeSpan duration;
	duration.Duration = this->player->GetDuration();
	return duration;
}

void Reader::EndOfRewindingTrack()
{
	
}

void Reader::EndOfPlayingTrack()	//����� �������� ������ ����� ���� � ��� �� ������
{
	Windows::Storage::Streams::IRandomAccessStream ^stream;
	MFAudioReader *reader = new MFAudioReader();
	int playlistLength = this->currentPlayList->GetPlayListLength();

	if (this->currentPlayList->CheckNext(this->trackNumber) && this->trackNumber<playlistLength)
	{
		stream = this->currentPlayList->GetStream(++this->trackNumber);
		reader->Initialize(stream);
		this->player->SetAudioData(reader, this->xAudio2);
	}
		
}

//���� ��� ��������������� ���������� ��������� ������ - ���� �� ������ �������, ���� ���� ������� - �� ������� �����, �������� � ������ � ������������� ��� � ������ �������(�������)
//����� �������������� ������ ����� � ������������� ��� � ������ �������(�������) (������ ������ ����� �� �����)
void Reader::IfMarkerMet()
{
	InitMasterVoice::GetInstance();
	Windows::Storage::Streams::IRandomAccessStream ^stream;
	//Marker marker;
	MFAudioReader *reader = new MFAudioReader();
	auto p = std::shared_ptr<XAudio2Player>(new XAudio2Player());
	this->xAudio2 = InitMasterVoice::GetInstance().GetXAudio();
	stream = this->currentPlayList->GetStream(this->trackNumber + 1);
	reader->Initialize(stream);
	p->Initialize(reader, this->xAudio2, this->events);

	this->playersList.push_back(p);
}

void Reader::Volume(float setVolume)
{
	this->player->SetVolume(setVolume);
}

LONGLONG Reader::CurrPos()
{
	return this->player->GetCurrentPosition();
}

void Reader::Stop()
{
	this->player->Stop();
}