#include "MetadataManager.h"
#include <QFileInfo>
#include <QDebug>

// TagLib includes
#include <fileref.h>
#include <tag.h>
#include <mpegfile.h>
#include <id3v2tag.h>
#include <attachedpictureframe.h>
#include <flacfile.h>
#include <flacpicture.h>

#ifdef _WIN32
#include <windows.h>
#endif

// Helper to convert QString to TagLib::FileName
static TagLib::FileName toFileName(const QString &path) {
    return TagLib::FileName(path.toUtf8().constData());
}

// Helper to convert TagLib::String to QString
static QString toQString(const TagLib::String &str) {
    return QString::fromUtf8(str.to8Bit(true).c_str());
}

// Helper to convert QString to TagLib::String
static TagLib::String toTagLibString(const QString &str) {
    return TagLib::String(str.toUtf8().constData(), TagLib::String::UTF8);
}

TrackMetadata MetadataManager::readMetadata(const QString &filePath) {
    TrackMetadata meta;
    meta.filePath = filePath;
    meta.title = QFileInfo(filePath).baseName(); // fallback

    try {
        {
            TagLib::FileRef f(toFileName(filePath));
            if (!f.isNull() && f.tag()) {
                TagLib::Tag *tag = f.tag();
                QString title = toQString(tag->title());
                if (!title.isEmpty()) meta.title = title;
                meta.artist = toQString(tag->artist());
                meta.album = toQString(tag->album());
                
                if (f.audioProperties()) {
                    meta.duration = f.audioProperties()->lengthInSeconds();
                }
            }
        } // FileRef f goes out of scope here and releases lock!

        // Read covers for MP3/FLAC
        QString ext = QFileInfo(filePath).suffix().toLower();
        if (ext == "mp3") {
            TagLib::MPEG::File mpegFile(toFileName(filePath));
            if (mpegFile.isValid() && mpegFile.ID3v2Tag()) {
                TagLib::ID3v2::Tag *id3v2 = mpegFile.ID3v2Tag();
                auto frameList = id3v2->frameListMap()["APIC"];
                if (frameList.isEmpty()) {
                    frameList = id3v2->frameListMap()["PIC"];
                }
                if (!frameList.isEmpty()) {
                    auto *frame = static_cast<TagLib::ID3v2::AttachedPictureFrame*>(frameList.front());
                    meta.coverData = QByteArray(frame->picture().data(), frame->picture().size());
                    meta.coverMimeType = toQString(frame->mimeType());
                }
            }
        } else if (ext == "flac") {
            TagLib::FLAC::File flacFile(toFileName(filePath));
            if (flacFile.isValid()) {
                auto picList = flacFile.pictureList();
                if (!picList.isEmpty()) {
                    auto *pic = picList.front();
                    meta.coverData = QByteArray(pic->data().data(), pic->data().size());
                    meta.coverMimeType = toQString(pic->mimeType());
                }
            }
        }
    } catch (const std::exception &e) {
        qWarning() << "Error reading metadata for" << filePath << ":" << e.what();
    } catch (...) {
        qWarning() << "Unknown error reading metadata for" << filePath;
    }

    return meta;
}

bool MetadataManager::writeMetadata(const QString &filePath, const QString &title, const QString &artist, const QByteArray &coverData) {
    try {
        {
            TagLib::FileRef f(toFileName(filePath));
            if (!f.isNull() && f.tag()) {
                TagLib::Tag *tag = f.tag();
                tag->setTitle(toTagLibString(title));
                tag->setArtist(toTagLibString(artist));
                f.save();
            }
        } // FileRef f goes out of scope here and releases lock!

        // Save cover if data is provided and file format matches
        if (!coverData.isEmpty()) {
            QString ext = QFileInfo(filePath).suffix().toLower();
            if (ext == "mp3") {
                TagLib::MPEG::File mpegFile(toFileName(filePath));
                if (mpegFile.isValid()) {
                    TagLib::ID3v2::Tag *id3v2 = mpegFile.ID3v2Tag(true);
                    id3v2->removeFrames("APIC");
                    id3v2->removeFrames("PIC");
                    auto *frame = new TagLib::ID3v2::AttachedPictureFrame();
                    frame->setMimeType("image/jpeg");
                    frame->setType(TagLib::ID3v2::AttachedPictureFrame::FrontCover);
                    frame->setPicture(TagLib::ByteVector(coverData.constData(), coverData.size()));
                    id3v2->addFrame(frame);
                    mpegFile.save();
                }
            } else if (ext == "flac") {
                TagLib::FLAC::File flacFile(toFileName(filePath));
                if (flacFile.isValid()) {
                    flacFile.removePictures();
                    auto *pic = new TagLib::FLAC::Picture();
                    pic->setMimeType("image/jpeg");
                    pic->setType(TagLib::FLAC::Picture::FrontCover);
                    pic->setData(TagLib::ByteVector(coverData.constData(), coverData.size()));
                    flacFile.addPicture(pic);
                    flacFile.save();
                }
            }
        }
        return true;
    } catch (const std::exception &e) {
        qWarning() << "Error writing metadata for" << filePath << ":" << e.what();
        return false;
    }
}
