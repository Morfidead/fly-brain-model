#include <iostream>
#include <fstream>
#include <unordered_map>
#include <cstdint>

struct NeuronRecord
{
    uint64_t root_id;
    uint32_t index;
};

int main()
{
    std::ifstream file(
        "neuron_index.bin",
        std::ios::binary
    );

    if (!file.is_open())
    {
        std::cout << "Не удалось открыть neuron_index.bin\n";
        return 1;
    }

    std::unordered_map<uint64_t, uint32_t> idToIndex;

    NeuronRecord neuron;

    while (file.read(
        reinterpret_cast<char*>(&neuron),
        sizeof(NeuronRecord)))
    {
        idToIndex[neuron.root_id] = neuron.index;
    }

    file.close();

    std::cout << "============================\n";
    std::cout << "     REAL FLYWIRE BRAIN\n";
    std::cout << "============================\n\n";

    std::cout
        << "Нейронов: "
        << idToIndex.size()
        << "\n\n";

    // Показываем несколько нейронов
    std::cout << "Первые нейроны:\n\n";

    for (uint64_t id = 0; id < 10; id++)
    {
        // Пока просто пропускаем —
        // unordered_map не хранит порядок.
    }

    // Проверяем конкретный настоящий root_id
    uint64_t testId = 720575940625363947;

    auto it = idToIndex.find(testId);

    if (it != idToIndex.end())
    {
        std::cout
            << "Найден нейрон:\n";

        std::cout
            << "root_id = "
            << testId
            << "\n";

        std::cout
            << "index = "
            << it->second
            << "\n";
    }
    else
    {
        std::cout
            << "Нейрон не найден\n";
    }

    return 0;
}