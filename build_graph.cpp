#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <cstdint>

struct BinaryConnection
{
    uint64_t pre_root_id;
    uint64_t post_root_id;

    uint16_t syn_count;

    uint8_t nt_type;

    // Название neuropil
    // Максимум 15 символов + '\0'
    char neuropil[16];
};

int main()
{
    // Большой CSV
    std::ifstream input(
        "/Users/mac/Downloads/connections_princeton_no_threshold.csv"
    );

    if (!input.is_open())
    {
        std::cout
            << "Не удалось открыть CSV\n";

        return 1;
    }

    // Новый бинарный файл
    std::ofstream output(
        "flywire_connections.bin",
        std::ios::binary
    );

    if (!output.is_open())
    {
        std::cout
            << "Не удалось создать бинарный файл\n";

        return 1;
    }

    std::string line;

    // Пропускаем заголовок
    std::getline(input, line);

    uint64_t count = 0;

    while (std::getline(input, line))
    {
        if (line.empty())
        {
            continue;
        }

        std::stringstream ss(line);

        std::string pre;
        std::string post;
        std::string neuropil;
        std::string syn;
        std::string nt;

        // Разбираем CSV
        std::getline(ss, pre, ',');
        std::getline(ss, post, ',');
        std::getline(ss, neuropil, ',');
        std::getline(ss, syn, ',');
        std::getline(ss, nt, ',');

        if (
            pre.empty() ||
            post.empty() ||
            syn.empty()
        )
        {
            continue;
        }

        BinaryConnection connection{};

        // ID нейронов
        connection.pre_root_id =
            std::stoull(pre);

        connection.post_root_id =
            std::stoull(post);

        // Количество синапсов
        connection.syn_count =
            static_cast<uint16_t>(
                std::stoi(syn)
            );

        // Нейромедиатор
        if (nt == "ACH")
        {
            connection.nt_type = 1;
        }
        else if (nt == "GABA")
        {
            connection.nt_type = 2;
        }
        else if (nt == "GLUT")
        {
            connection.nt_type = 3;
        }
        else
        {
            connection.nt_type = 0;
        }

        // ====================================
        // Сохраняем neuropil
        // ====================================

        std::strncpy(
            connection.neuropil,
            neuropil.c_str(),
            sizeof(connection.neuropil) - 1
        );

        // Гарантируем '\0' в конце
        connection.neuropil[
            sizeof(connection.neuropil) - 1
        ] = '\0';

        // Записываем связь
        output.write(
            reinterpret_cast<const char*>(
                &connection
            ),
            sizeof(BinaryConnection)
        );

        count++;

        if (count % 1000000 == 0)
        {
            std::cout
                << "Обработано: "
                << count
                << " связей\n";
        }
    }

    input.close();
    output.close();

    std::cout
        << "\nГотово!\n";

    std::cout
        << "Связей записано: "
        << count
        << "\n";

    std::cout
        << "Размер одной записи: "
        << sizeof(BinaryConnection)
        << " байт\n";

    return 0;
}