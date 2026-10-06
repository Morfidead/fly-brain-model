#include <iostream>
#include <fstream>
#include <cstdint>

struct BinaryConnection
{
    uint64_t pre_root_id;
    uint64_t post_root_id;

    uint16_t syn_count;

    uint8_t nt_type;
};

std::string getNeurotransmitter(uint8_t type)
{
    if (type == 1)
        return "ACH";

    if (type == 2)
        return "GABA";

    if (type == 3)
        return "GLUT";

    return "UNKNOWN";
}

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

    std::cout << "Первые связи FlyWire:\n\n";

    for (int i = 0; i < 10; i++)
    {
        BinaryConnection connection;

        file.read(
            reinterpret_cast<char*>(&connection),
            sizeof(BinaryConnection)
        );

        if (!file)
        {
            std::cout << "Достигнут конец файла\n";
            break;
        }

        std::cout
            << "Связь #" << i + 1 << "\n";

        std::cout
            << "  FROM: "
            << connection.pre_root_id
            << "\n";

        std::cout
            << "  TO:   "
            << connection.post_root_id
            << "\n";

        std::cout
            << "  Synapses: "
            << connection.syn_count
            << "\n";

        std::cout
            << "  NT: "
            << getNeurotransmitter(connection.nt_type)
            << "\n\n";
    }

    return 0;
}