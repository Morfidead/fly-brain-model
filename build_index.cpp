#include <iostream>
#include <fstream>
#include <unordered_map>
#include <cstdint>

struct BinaryConnection
{
    uint64_t pre_root_id;
    uint64_t post_root_id;
    uint16_t syn_count;
    uint8_t nt_type;
};

struct NeuronRecord
{
    uint64_t root_id;
    uint32_t index;
};

int main()
{
    std::ifstream file(
        "flywire_connections.bin",
        std::ios::binary
    );

    if (!file.is_open())
    {
        std::cout << "Не удалось открыть бинарный файл\n";
        return 1;
    }

    // root_id -> индекс
    std::unordered_map<uint64_t, uint32_t> idToIndex;

    // Будем записывать нейроны сюда
    std::ofstream indexFile(
        "neuron_index.bin",
        std::ios::binary
    );

    if (!indexFile.is_open())
    {
        std::cout << "Не удалось создать neuron_index.bin\n";
        return 1;
    }

    uint32_t nextIndex = 0;
    uint64_t connectionCount = 0;

    BinaryConnection connection;

    while (file.read(
        reinterpret_cast<char*>(&connection),
        sizeof(BinaryConnection)))
    {
        // Обрабатываем PRE нейрон
        if (!idToIndex.contains(connection.pre_root_id))
        {
            uint32_t index = nextIndex++;

            idToIndex[connection.pre_root_id] = index;

            NeuronRecord neuron{
                connection.pre_root_id,
                index
            };

            indexFile.write(
                reinterpret_cast<const char*>(&neuron),
                sizeof(NeuronRecord)
            );
        }

        // Обрабатываем POST нейрон
        if (!idToIndex.contains(connection.post_root_id))
        {
            uint32_t index = nextIndex++;

            idToIndex[connection.post_root_id] = index;

            NeuronRecord neuron{
                connection.post_root_id,
                index
            };

            indexFile.write(
                reinterpret_cast<const char*>(&neuron),
                sizeof(NeuronRecord)
            );
        }

        connectionCount++;

        if (connectionCount % 1000000 == 0)
        {
            std::cout
                << "Обработано связей: "
                << connectionCount
                << "\n";
        }
    }

    file.close();
    indexFile.close();

    std::cout << "\n============================\n";
    std::cout << "ИНДЕКС ПОСТРОЕН\n";
    std::cout << "============================\n";

    std::cout
        << "Связей: "
        << connectionCount
        << "\n";

    std::cout
        << "Нейронов: "
        << nextIndex
        << "\n";

    std::cout
        << "Размер записи нейрона: "
        << sizeof(NeuronRecord)
        << " байт\n";

    return 0;
}