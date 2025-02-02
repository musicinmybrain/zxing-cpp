/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "ODRSSFieldParser.h"

#include "DecodeStatus.h"
#include "ZXContainerAlgorithms.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iterator>

namespace ZXing::OneD::DataBar {

	//private static final Object VARIABLE_LENGTH = new Object();

struct AiInfo
{
	const char* aiPrefix;
	int fieldSize;	// if negative, the length is variable and abs(length) give the max size
};

// GS1 General Specifications Release 22.0 (Jan 22, 2022)
static const AiInfo aiInfos[] = {
// TWO_DIGIT_DATA_LENGTH
	{ "00", 18 },
	{ "01", 14 },
	{ "02", 14 },

	{ "10", -20 },
	{ "11", 6 },
	{ "12", 6 },
	{ "13", 6 },
	{ "15", 6 },
	{ "16", 6 },
	{ "17", 6 },

	{ "20", 2 },
	{ "21", -20 },
	{ "22", -20 },

	{ "30", -8 },
	{ "37", -8 },

	//internal company codes
	{ "90", -30 },
	{ "91", -90 },
	{ "92", -90 },
	{ "93", -90 },
	{ "94", -90 },
	{ "95", -90 },
	{ "96", -90 },
	{ "97", -90 },
	{ "98", -90 },
	{ "99", -90 },

//THREE_DIGIT_DATA_LENGTH
	{ "235", -28 },
	{ "240", -30 },
	{ "241", -30 },
	{ "242", -6 },
	{ "243", -20 },
	{ "250", -30 },
	{ "251", -30 },
	{ "253", -30 },
	{ "254", -20 },
	{ "255", -25 },

	{ "400", -30 },
	{ "401", -30 },
	{ "402", 17 },
	{ "403", -30 },
	{ "410", 13 },
	{ "411", 13 },
	{ "412", 13 },
	{ "413", 13 },
	{ "414", 13 },
	{ "415", 13 },
	{ "416", 13 },
	{ "417", 13 },
	{ "420", -20 },
	{ "421", -12 },
	{ "422", 3 },
	{ "423", -15 },
	{ "424", 3 },
	{ "425", -15 },
	{ "426", 3 },
	{ "427", -3 },

	{ "710", -20 },
	{ "711", -20 },
	{ "712", -20 },
	{ "713", -20 },
	{ "714", -20 },
	{ "715", -20 },

//THREE_DIGIT_PLUS_DIGIT_DATA_LENGTH
	{ "310", 6 },
	{ "311", 6 },
	{ "312", 6 },
	{ "313", 6 },
	{ "314", 6 },
	{ "315", 6 },
	{ "316", 6 },
	{ "320", 6 },
	{ "321", 6 },
	{ "322", 6 },
	{ "323", 6 },
	{ "324", 6 },
	{ "325", 6 },
	{ "326", 6 },
	{ "327", 6 },
	{ "328", 6 },
	{ "329", 6 },
	{ "330", 6 },
	{ "331", 6 },
	{ "332", 6 },
	{ "333", 6 },
	{ "334", 6 },
	{ "335", 6 },
	{ "336", 6 },
	{ "337", 6 },
	{ "340", 6 },
	{ "341", 6 },
	{ "342", 6 },
	{ "343", 6 },
	{ "344", 6 },
	{ "345", 6 },
	{ "346", 6 },
	{ "347", 6 },
	{ "348", 6 },
	{ "349", 6 },
	{ "350", 6 },
	{ "351", 6 },
	{ "352", 6 },
	{ "353", 6 },
	{ "354", 6 },
	{ "355", 6 },
	{ "356", 6 },
	{ "357", 6 },
	{ "360", 6 },
	{ "361", 6 },
	{ "362", 6 },
	{ "363", 6 },
	{ "364", 6 },
	{ "365", 6 },
	{ "366", 6 },
	{ "367", 6 },
	{ "368", 6 },
	{ "369", 6 },
	{ "390", -15 },
	{ "391", -18 },
	{ "392", -15 },
	{ "393", -18 },
	{ "394", 4 },
	{ "395", 6 },
	{ "703", -30 },
	{ "723", -30 },

//FOUR_DIGIT_DATA_LENGTH
	{ "4300", -35 },
	{ "4301", -35 },
	{ "4302", -70 },
	{ "4303", -70 },
	{ "4304", -70 },
	{ "4305", -70 },
	{ "4306", -70 },
	{ "4307", 2 },
	{ "4308", -30 },
	{ "4310", -35 },
	{ "4311", -35 },
	{ "4312", -70 },
	{ "4313", -70 },
	{ "4314", -70 },
	{ "4315", -70 },
	{ "4316", -70 },
	{ "4317", 2 },
	{ "4318", -20 },
	{ "4319", -30 },
	{ "4320", -35 },
	{ "4321", 1 },
	{ "4322", 1 },
	{ "4323", 1 },
	{ "4324", 10 },
	{ "4325", 10 },
	{ "4326", 6 },

	{ "7001", 13 },
	{ "7002", -30 },
	{ "7003", 10 },
	{ "7004", -4 },
	{ "7005", -12 },
	{ "7006", 6 },
	{ "7007", -12 },
	{ "7008", -3 },
	{ "7009", -10 },
	{ "7010", -2 },
	{ "7020", -20 },
	{ "7021", -20 },
	{ "7022", -20 },
	{ "7023", -30 },
	{ "7040", 4 },
	{ "7240", -20 },

	{ "8001", 14 },
	{ "8002", -20 },
	{ "8003", -30 },
	{ "8004", -30 },
	{ "8005", 6 },
	{ "8006", 18 },
	{ "8007", -34 },
	{ "8008", -12 },
	{ "8009", -50 },
	{ "8010", -30 },
	{ "8011", -12 },
	{ "8012", -20 },
	{ "8013", -25 },
	{ "8017", 18 },
	{ "8018", 18 },
	{ "8019", -10 },
	{ "8020", -25 },
	{ "8026", 18 },
	{ "8110", -70 },
	{ "8111", 4 },
	{ "8112", -70 },
	{ "8200", -70 },
};

static size_t
AiSize(const char* aiPrefix)
{
	if ((aiPrefix[0] == '3' && Contains("1234569", aiPrefix[1])) || std::string(aiPrefix) == "703"
			|| std::string(aiPrefix) == "723")
		return 4;
	else
		return strlen(aiPrefix);
}


DecodeStatus
ParseFieldsInGeneralPurpose(const std::string &rawInfo, std::string& result)
{
	if (rawInfo.empty()) {
		return DecodeStatus::NoError;
	}

	auto starts_with =
		[](const std::string& str, const char* pre) { return strncmp(pre, str.data(), strlen(pre)) == 0; };

	const AiInfo* aiInfo = FindIf(aiInfos, [&](const AiInfo& i) { return starts_with(rawInfo, i.aiPrefix); });
	if (aiInfo == std::end(aiInfos))
		return DecodeStatus::NotFound;

	size_t aiSize = AiSize(aiInfo->aiPrefix);

	// require at least one character in the variable field size case
	if (rawInfo.length() < aiSize + std::max(1, aiInfo->fieldSize))
		return DecodeStatus::NotFound;

	size_t fieldSize = aiInfo->fieldSize >= 0
						   ? size_t(aiInfo->fieldSize)                                        // fixed
						   : std::min(rawInfo.length() - aiSize, size_t(-aiInfo->fieldSize)); // variable

	auto ai = rawInfo.substr(0, aiSize);
	auto field = rawInfo.substr(aiSize, fieldSize);
	auto remaining = rawInfo.substr(aiSize + fieldSize);
	std::string parsedRemaining;
	auto status = ParseFieldsInGeneralPurpose(remaining, parsedRemaining);
	result = '(' + ai + ')' + field + parsedRemaining;
	return status;
}

} // namespace ZXing::OneD::DataBar
