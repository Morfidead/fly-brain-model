#include <iostream>
#include "fly_brain.h"

int main()
{
    FlyBrain brain;

    std::cout << "============================\n";
    std::cout << "       FLYWIRE BRAIN\n";
    std::cout << "============================\n\n";


    // 1. Загружаем список всех нейронов
    if (!brain.loadNeuronIndex("neuron_index.bin"))
    {
        std::cout
            << "Не удалось открыть neuron_index.bin\n";

        return 1;
    }

    std::cout
        << "Нейронов: "
        << brain.getNeuronCount()
        << "\n\n";


    // 2. Загружаем связи между нейронами
    std::cout
        << "Загрузка связей...\n";

    if (!brain.loadConnections(
        "flywire_connections.bin"
    ))
    {
        std::cout
            << "Не удалось открыть flywire_connections.bin\n";

        return 1;
    }

    std::cout
        << "\nСвязи загружены.\n";


    // 3. Загружаем информацию о типах нейронов
    std::cout
        << "Загрузка аннотаций...\n";

    brain.loadNeuronAnnotations(
        "data/visual_neuron_types (1).csv"
    );
brain.loadNeuronAnnotations(
    "data/visual_neuron_types (1).csv"
);

std::cout
    << "Аннотации загружены.\n";

std::cout
    << "\n============================\n"
    << "ПОИСК DNa02\n"
    << "============================\n";

NeuronId dNa02L =
    720575940629327659ULL;

NeuronId dNa02R =
    720575940604737708ULL;

std::cout
    << "DNa02 LEFT:\n"
    << "  ID = "
    << dNa02L
    << "\n"
    << "  type = "
    << brain.getNeuronType(dNa02L)
    << "\n\n";

std::cout
    << "DNa02 RIGHT:\n"
    << "  ID = "
    << dNa02R
    << "\n"
    << "  type = "
    << brain.getNeuronType(dNa02R)
    << "\n";g++ -std=c++17 main.cpp fly_brain.cpp -o fly_brain


    // 4. Наш настоящий R7-фоторецептор
    NeuronId r7 =
        720575940605179360;
    float visualSignal = 1.0f;
    std::cout
    << "\n============================\n"
    << "ПРОВЕРКА ПУТИ R7\n"
    << "============================\n";

brain.traceForward(r7, 10);

    std::cout
        << "Аннотации загружены.\n";
    brain.printNeuronTypes();
std::cout
    << "\n============================\n"
    << "ПРОВЕРКА ПУТИ R7\n"
    << "============================\n";

brain.traceForward(r7, 10);


for (int frame = 0; frame < 5; frame++)
{
    std::cout
        << "\n\n============================\n";

    std::cout
        << "VISUAL FRAME "
        << frame
        << "\n";

    std::cout
        << "============================\n";


    // Новый кадр
    brain.resetActivations();


    // Ввод от фоторецептора
    brain.setInput(
        r7,
        visualSignal
    );


    std::cout
        << "R7 signal: "
        << visualSignal
        << "\n";


    // Пусть сигнал проходит несколько уровней мозга
    for (int step = 0; step < 5; step++)
    {
        brain.update();
    }


    // Показываем результат
    brain.printActiveNeurons();


    // В следующем кадре немного меняем изображение
    visualSignal *= 0.8f;
}


    std::cout
        << "\n============================\n";

    std::cout
        << "R7-ФОТОРЕЦЕПТОР\n";

    std::cout
        << "============================\n";

    std::cout
        << "root_id: "
        << r7
        << "\n";

    std::cout
        << "type: "
        << brain.getNeuronType(r7)
        << "\n";


    

    return 0;
}
