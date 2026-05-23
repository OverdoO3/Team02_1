#include "Misc.h"
#include "System/AudioResource.h"
#include <string>

// WAVEタグ作成マクロ
#define MAKE_WAVE_TAG_VALUE(c1, c2, c3, c4)  ( c1 | (c2<<8) | (c3<<16) | (c4<<24) )

// コンストラクタ
AudioResource::AudioResource(const char* filename)
{
    FILE* fp = nullptr;
    errno_t error = fopen_s(&fp, filename, "rb");

    // ファイルが開けない場合、無理に読み込まず空のまま返す（プログラムを落とさない）
    if (error != 0 || fp == nullptr)
    {
        // デバッグ出力で失敗したパスを確認できるようにする
        OutputDebugStringA(("!!! AudioResource: File not found -> " + std::string(filename) + "\n").c_str());
        return;
    }

    // --- 以下、ファイルが開けた場合のみ実行される ---

    // ファイルのサイズを求める
    fseek(fp, 0, SEEK_END);
    size_t size = static_cast<size_t>(ftell(fp));
    fseek(fp, 0, SEEK_SET);

    size_t readBytes = 0;

    // RIFFヘッダ
    fread(&riff, sizeof(riff), 1, fp);
    readBytes += sizeof(riff);

    // "RIFF" と "WAVE" のフォーマットチェック
    if (riff.tag != MAKE_WAVE_TAG_VALUE('R', 'I', 'F', 'F') ||
        riff.type != MAKE_WAVE_TAG_VALUE('W', 'A', 'V', 'E'))
    {
        fclose(fp);
        OutputDebugStringA("!!! AudioResource: Invalid format\n");
        return;
    }

    while (size > readBytes)
    {
        Chunk chunk;
        fread(&chunk, sizeof(chunk), 1, fp);
        readBytes += sizeof(chunk);

        // 'fmt '
        if (chunk.tag == MAKE_WAVE_TAG_VALUE('f', 'm', 't', ' '))
        {
            fread(&fmt, sizeof(fmt), 1, fp);
            readBytes += sizeof(fmt);

            if (chunk.size > sizeof(Fmt))
            {
                UINT16 extSize;
                fread(&extSize, sizeof(extSize), 1, fp);
                readBytes += sizeof(extSize);
                if (readBytes + chunk.size == size) break;
                fseek(fp, extSize, SEEK_CUR);
                readBytes += extSize;
            }
        }
        // 'data'
        else if (chunk.tag == MAKE_WAVE_TAG_VALUE('d', 'a', 't', 'a'))
        {
            data.resize(chunk.size);
            fread(data.data(), chunk.size, 1, fp);
            readBytes += chunk.size;

            if (fmt.quantumBits == 8)
            {
                for (UINT32 i = 0; i < chunk.size; ++i) data[i] -= 128;
            }
        }
        // その他
        else
        {
            if (readBytes + chunk.size == size) break;
            fseek(fp, chunk.size, SEEK_CUR);
            readBytes += chunk.size;
        }
    }

    fclose(fp);

	// WAV フォーマットをセットアップ
	{
		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nChannels = fmt.channel;
		wfx.nSamplesPerSec = fmt.sampleRate;
		wfx.wBitsPerSample = fmt.quantumBits;
		wfx.nBlockAlign = (wfx.wBitsPerSample >> 3) * wfx.nChannels;
		wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
		wfx.cbSize = sizeof(WAVEFORMATEX);
	}
}

// デストラクタ
AudioResource::~AudioResource()
{

}
