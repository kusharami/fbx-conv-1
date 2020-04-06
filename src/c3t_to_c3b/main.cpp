#include "modeldata/Model.h"
#include "modeldata/Node.h"
#include "modeldata/C3BFile.h"

#include <QCoreApplication>
#include <QCommandLineParser>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>

using namespace fbxconv;
using namespace modeldata;

static const QString C3B_EXT = QStringLiteral(".c3b");

static int badInputFile()
{
	qCritical("Bad input c3t file.");
	return 2;
}

struct Bone : public NodeAnimation
{
	Node mutableNode;
	Bone()
	{
		node = &mutableNode;
	}
};

static Keyframe *keyframeWithEntry(const QJsonValue& entry)
{
	if (!entry.isObject())
	{
		return nullptr;
	}

	auto obj = entry.toObject();

	auto rotation = obj.value("rotation");
	if (!rotation.isArray() && !rotation.isUndefined())
	{
		return nullptr;
	}
	auto scale = obj.value("scale");
	if (!scale.isArray() && !scale.isUndefined())
	{
		return nullptr;
	}
	auto translation = obj.value("translation");
	if (!translation.isArray() && !translation.isUndefined())
	{
		return nullptr;
	}

	auto keytime = obj.value("keytime");
	if (!keytime.isDouble())
	{
		return nullptr;
	}

	auto ra = rotation.toArray();
	if (!rotation.isUndefined() && ra.size() != 4)
	{
		return nullptr;
	}

	auto sa = scale.toArray();
	if (!scale.isUndefined() && sa.size() != 3)
	{
		return nullptr;
	}

	auto ta = translation.toArray();
	if (!translation.isUndefined() && ta.size() != 3)
	{
		return nullptr;
	}

	QScopedPointer<Keyframe> keyframe(new Keyframe);
	keyframe->time = keytime.toDouble();
	if ((keyframe->hasRotation = !ra.isEmpty()))
	{
		for (int i = 0; i < 4; i++)
		{
			keyframe->rotation[i] = float(ra.at(i).toDouble());
		}
	}

	if ((keyframe->hasScale = !sa.isEmpty()))
	{
		for (int i = 0; i < 3; i++)
		{
			keyframe->scale[i] = float(sa.at(i).toDouble());
		}
	}

	if ((keyframe->hasTranslation = !ta.isEmpty()))
	{
		for (int i = 0; i < 3; i++)
		{
			keyframe->translation[i] = float(ta.at(i).toDouble());
		}
	}

	return keyframe.take();
}

static Bone *boneWithEntry(const QJsonValue& entry)
{
	if (!entry.isObject())
	{
		return nullptr;
	}

	auto obj = entry.toObject();

	auto keyframes = obj.value("keyframes");
	if (!keyframes.isArray())
	{
		return nullptr;
	}

	auto boneId = obj.value("boneId");
	if (!boneId.isString())
	{
		return nullptr;
	}

	QScopedPointer<Bone> bone(new Bone);
	bone->mutableNode.id = boneId.toString().toUtf8().toStdString();

	auto ka = keyframes.toArray();
	bone->keyframes.reserve(ka.count());
	for (auto it = ka.constBegin(); it != ka.constEnd(); ++it)
	{
		auto keyframe = keyframeWithEntry(*it);
		if (!keyframe)
			return nullptr;

		bone->keyframes.push_back(keyframe);
	}

	return bone.take();
}

static Animation *animationWithEntry(const QJsonValue& entry)
{
	if (!entry.isObject())
	{
		return nullptr;
	}

	auto obj = entry.toObject();

	auto bones = obj.value("bones");
	if (!bones.isArray())
	{
		return nullptr;
	}

	auto id = obj.value("id");
	if (!id.isString())
	{
		return nullptr;
	}

	auto length = obj.value("length");
	if (!length.isDouble())
	{
		return nullptr;
	}

	QScopedPointer<Animation> anim(new Animation);
	anim->id = id.toString().toUtf8().toStdString();
	anim->length = length.toDouble();

	auto ba = bones.toArray();
	anim->nodeAnimations.reserve(ba.count());
	for (auto it = ba.constBegin(); it != ba.constEnd(); ++it)
	{
		auto bone = boneWithEntry(*it);
		if (!bone)
			return nullptr;

		anim->nodeAnimations.push_back(bone);
	}

	return anim.take();
}

bool parseAnimations(Model* to, const QJsonArray &from)
{
	Q_ASSERT(to);
	to->animations.clear();
	to->animations.reserve(from.count());
	for (auto it = from.constBegin(); it != from.constEnd(); ++it)
	{
		auto anim = animationWithEntry(*it);
		if (!anim)
		{
			return false;
		}

		to->animations.push_back(anim);
	}

	return true;
}

bool saveModel(Model *model, const QString &filePath)
{
	C3BFile outputFile;
	outputFile.AddModel(model);
	return outputFile.saveBinary(filePath.toUtf8().toStdString());
}

static int saveOutputFileError()
{
	qCritical("Unable to save output c3b file.");
	return 4;
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	QCoreApplication::setApplicationName("c3t_to_c3b");
	QCoreApplication::setApplicationVersion(APP_VERSION);

	QCommandLineParser parser;

	parser.setApplicationDescription(
				QString("Convert c3t to c3b. "
						"Version %1 supports animations only.").arg(
					QLatin1String(APP_VERSION)));
	parser.addVersionOption();
	parser.addHelpOption();
	QCommandLineOption inputOption(
		{ "i", "input" }, "Input c3t file path.", "file");
	QCommandLineOption outputOption(
		{ "o", "output" }, "Output c3b file path.", "file");
	QCommandLineOption separateAnimOption(
		 "separate-anim", "Write each animation in a separate file.");

	parser.addOption(inputOption);
	parser.addOption(outputOption);
	parser.addOption(separateAnimOption);

	parser.process(a);

	auto inputFilePath = parser.value(inputOption);
	auto outputFilePath = parser.value(outputOption);
	if (inputFilePath.isEmpty() || outputFilePath.isEmpty())
	{
		qCritical("Please enter input and output file paths.");
		return 5;
	}

	bool separateAnimFiles = parser.isSet(separateAnimOption);

	QJsonDocument input;
	{
		QFile file(inputFilePath);
		if (!file.open(QFile::ReadOnly))
		{
			qCritical("Failed to open input c3t file.");
			return 1;
		}
		{
			QJsonParseError result;
			input = input.fromJson(file.read(100 * 1024 * 1024), &result);
			if (result.error != QJsonParseError::NoError)
			{
				qCritical().noquote() << result.errorString();
				return 2;
			}

			if (!input.isObject())
			{
				return badInputFile();
			}
		}
	}

	auto obj = input.object();
	auto version = obj.value("version").toString().split('.');
	bool ok = false;
	Model model;
	if (version.size() == 2)
	{
		model.version[0] = short(version[0].toUInt(&ok));
		if (ok)
			model.version[1] = short(version[1].toUInt(&ok));
	}
	if (!ok
	||	model.version[0] != VERSION_HI
	||	model.version[1] != VERSION_LO)
	{
		qCritical("Bad input c3t file version, %d.%d expected.",
				  VERSION_HI, VERSION_LO);
		return 3;
	}

	auto animations = obj.value("animations");
	if (!animations.isArray() && !animations.isUndefined())
	{
		return badInputFile();
	}

	model.exportPart = EXPORT_PART_ANIMATION;

	if (!parseAnimations(&model, animations.toArray()))
		return badInputFile();

	if (separateAnimFiles)
	{
		if (outputFilePath.endsWith(C3B_EXT, Qt::CaseInsensitive))
		{
			outputFilePath = outputFilePath.left(
						outputFilePath.length() - C3B_EXT.length());
		}

		auto allAnimations = std::move(model.animations);

		bool ok = true;
		for (auto anim : allAnimations)
		{
			model.animations = { anim };
			if (!saveModel(&model,
				outputFilePath + '_' + QString::fromUtf8(
				QByteArray(anim->id.c_str(), int(anim->id.size()))) + C3B_EXT))
			{
				ok = false;
				break;
			}
		}

		model.animations = std::move(allAnimations);
		if (!ok)
			return saveOutputFileError();
	} else
	{
		if (!outputFilePath.endsWith(C3B_EXT, Qt::CaseInsensitive))
		{
			outputFilePath += C3B_EXT;
		}

		if (!saveModel(&model, outputFilePath))
			return saveOutputFileError();
	}

	return 0;
}
