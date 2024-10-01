#include <nlohmann/json.hpp>

#include "Serialization/SerializationEntry.h"
#include "Wheeler/Wheeler.h"

namespace Serial {
    bool Write(const SKSE::SerializationInterface* a_interface, const std::string& data) {
        const std::size_t  size{ data.length() };
        constexpr uint32_t maxChunkSize{ std::numeric_limits<uint32_t>::max() };

        logger::trace("Writing string data of size: {}", size);

        if(!a_interface->WriteRecordData(size)) {
            logger::error("Failed to write size of string data");
            return false;
        }

        size_t offset{};

        while(offset < size) {
            const uint32_t bytesToSend{ std::min(static_cast<uint32_t>(size - offset), maxChunkSize) };
            logger::trace("Writing chunk of size: {} at offset: {}", bytesToSend, offset);
            if(!a_interface->WriteRecordData(data.data() + offset, bytesToSend)) {
                logger::error("Failed to write data chunk at offset: {}", offset);
                return false;
            }
            offset += bytesToSend;
        }
        logger::trace("Successfully wrote string data of total size: {}", size);
        return true;
    }

    bool Write(const SKSE::SerializationInterface* a_interface, const std::vector<uint8_t>& data) {
        const size_t       size{ data.size() };
        constexpr uint32_t maxChunkSize{ std::numeric_limits<uint32_t>::max() };

        logger::trace("Writing json msgpack data of size: {}", size);

        if(!a_interface->WriteRecordData(size)) {
            logger::error("Failed to write size of json msgpack data");
            return false;
        }

        size_t offset{};

        while(offset < size) {
            const uint32_t bytesToSend{ std::min(static_cast<uint32_t>(size - offset), maxChunkSize) };
            logger::trace("Writing chunk of size: {} at offset: {}", bytesToSend, offset);
            if(!a_interface->WriteRecordData(data.data() + offset, bytesToSend)) {
                logger::error("Failed to write data chunk at offset: {}", offset);
                return false;
            }
            offset += bytesToSend;
        }
        logger::trace("Successfully wrote json msgpack data of total size: {}", size);
        return true;
    }

    bool Read(const SKSE::SerializationInterface* a_interface, std::string& result) {
        std::size_t        size{};
        constexpr uint32_t maxChunkSize{ std::numeric_limits<uint32_t>::max() };

        if(!a_interface->ReadRecordData(size)) {
            logger::error("Failed to read size of string data");
            return false;
        }

        logger::trace("reading string data of size: {}", size);

        if(size > 0) {
            result.resize(size);
            std::size_t offset{};
            while(offset < size) {
                const uint32_t bytesToRead{ std::min(static_cast<uint32_t>(size - offset), maxChunkSize) };
                logger::trace("reading chunk of size: {} at offset: {}", bytesToRead, offset);
                if(!a_interface->ReadRecordData(&result[offset], bytesToRead)) {
                    logger::error("Failed to read data chunk at offset: {}", offset);
                    return false;
                }
                offset += bytesToRead;
            }
        } else {
            result.clear();
        }
        logger::trace("Successfully read string data of total size: {}", size);
        return true;
    }

    bool Read(const SKSE::SerializationInterface* a_interface, std::vector<uint8_t>& result) {
        std::size_t        size{};
        constexpr uint32_t maxChunkSize{ std::numeric_limits<uint32_t>::max() };

        if(!a_interface->ReadRecordData(size)) {
            logger::error("Failed to read size of json msgpack data");
            return false;
        }
        logger::trace("reading json msgpack data of size: {}", size);

        if(size > 0) {
            result.resize(size);
            std::size_t offset{};
            while(offset < size) {
                const uint32_t bytesToRead{ std::min(static_cast<uint32_t>(size - offset), maxChunkSize) };
                logger::trace("reading chunk of size: {} at offset: {}", bytesToRead, offset);
                if(!a_interface->ReadRecordData(&result[offset], bytesToRead)) {
                    logger::error("Failed to read data chunk at offset: {}", offset);
                    return false;
                }
                offset += bytesToRead;
            }
        } else {
            result.clear();
        }
        logger::trace("Successfully read json msgpack data of total size: {}", size);
        return true;
    }
}  // namespace Serial

void SerializationEntry::BindSerializationCallbacks(const SKSE::SerializationInterface* a_in) {
    a_in->SetLoadCallback(SerializationEntry::Load);
    a_in->SetSaveCallback(SerializationEntry::Save);
    a_in->SetRevertCallback(SerializationEntry::Revert);
}

void SerializationEntry::Save(SKSE::SerializationInterface* a_intfc) {
    logger::info("Serializing wheel into save...");
    if(!a_intfc->OpenRecord(WHEELER_JSON_MSGPACK_TYPE, WHEELER_JSON_MSGPACK_SERIALIZER_VERSION)) {
        logger::error("Failed to open record");
        return;
    }
    nlohmann::json j_wheeler;
    Wheeler::SerializeIntoJsonObj(j_wheeler);
    {
        const auto writeBuffer{ nlohmann::json::to_msgpack(j_wheeler) };
        // logger::info("Serializing following record: {}", writeBuffer);

        Serial::Write(a_intfc, writeBuffer);
    }
}

void SerializationEntry::Load(SKSE::SerializationInterface* a_intfc) {
    std::uint32_t type, version, length;
    if(!a_intfc->GetNextRecordInfo(type, version, length)) {
        logger::error("Failed to obtain record, abort loading");
        return;
    }
    if(type == WHEELER_JSON_STRING_TYPE && version == WHEELER_JSON_STRING_SERIALIZER_VERSION) {
        logger::info("Load: only for legacy purposes till dmenu v1.2");
        std::string readBuffer;
        Serial::Read(a_intfc, readBuffer);
        logger::trace("Read str: {}", readBuffer);
        try {
            const nlohmann::json j_wheeler(nlohmann::json::parse(readBuffer));
            Wheeler::Clear();
            Wheeler::SerializeFromJsonObj(j_wheeler, a_intfc);
        } catch(const nlohmann::json::exception& e) {
            logger::error("Failed to parse json: {}", e.what());
        }
    } else if(type == WHEELER_JSON_MSGPACK_TYPE && version == WHEELER_JSON_MSGPACK_SERIALIZER_VERSION) {
        std::vector<uint8_t> readBuffer;

        Serial::Read(a_intfc, readBuffer);
        try {
            const nlohmann::json j_wheeler(nlohmann::json::from_msgpack(readBuffer));
            Wheeler::Clear();
            Wheeler::SerializeFromJsonObj(j_wheeler, a_intfc);
        } catch(const nlohmann::json::exception& e) {
            logger::error("Failed to parse json: {}", e.what());
        }
    } else {
        logger::error("Load: Unsupported");
    }
}

void SerializationEntry::Revert(SKSE::SerializationInterface*) {
    Wheeler::Clear();
}

// serialization example:
/**

"activeWheel" : 0,
"wheels" :
[
	{
		"entries":
		[
			{
				"selectedItem" ： 3
				"items:
				[
					{
						"type": "WheelItemWeapon",
						"formID": 1234(uint),
						"uniqueID": 5(uint)							
					},
					{
						"type": "WheelItemArmor",
						"formID": 1324(uint),
						"uniqueID": 6(uint)
					},
					{
						"type": "WheelItemSpell",
						"formID": 1234(uint),
					}
				]
			},
			{
				"items:
				[
				]
			},
		]
	},
	{
		"entries":
		[
			{
				"items:
				[
				]
			},
		]
	},
]
	

*/
