#include "fly_brain.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <unordered_set>


// ============================================================
// КОНСТРУКТОР
// ============================================================

FlyBrain::FlyBrain()
{
}


// ============================================================
// ЗАГРУЗКА ИНДЕКСА НЕЙРОНОВ
// ============================================================

bool FlyBrain::loadNeuronIndex(
    const std::string& filename
)
{
    std::ifstream input(
        filename,
        std::ios::binary
    );

    if (!input.is_open())
    {
        return false;
    }


    struct NeuronRecord
    {
        uint64_t root_id;
        uint32_t index;
    };


    neuronIds.clear();
    idToIndex.clear();


    NeuronRecord record;


    while (
        input.read(
            reinterpret_cast<char*>(&record),
            sizeof(record)
        )
    )
    {
        uint32_t index =
            static_cast<uint32_t>(
                neuronIds.size()
            );


        neuronIds.push_back(
            record.root_id
        );


        idToIndex[
            record.root_id
        ] = index;
    }


    input.close();


    activation.assign(
        neuronIds.size(),
        0.0f
    );


    lastSource.assign(
        neuronIds.size(),
        -1
    );


    neuronInfo.resize(
        neuronIds.size()
    );


    // Создаём индекс исходящих связей
    outgoingConnections.resize(
        neuronIds.size()
    );


    return true;
}


// ============================================================
// ЗАГРУЗКА СВЯЗЕЙ
// ============================================================

bool FlyBrain::loadConnections(
    const std::string& filename
)
{
    std::ifstream input(
        filename,
        std::ios::binary
    );


    if (!input.is_open())
    {
        return false;
    }


    connections.clear();


    BinaryConnection binary;


    uint64_t count = 0;
    uint64_t skipped = 0;


    while (
        input.read(
            reinterpret_cast<char*>(&binary),
            sizeof(binary)
        )
    )
    {
        auto fromIt =
            idToIndex.find(
                binary.pre_root_id
            );


        auto toIt =
            idToIndex.find(
                binary.post_root_id
            );


        if (
            fromIt == idToIndex.end() ||
            toIt == idToIndex.end()
        )
        {
            skipped++;
            continue;
        }


        Connection connection{};


        connection.from =
            fromIt->second;


        connection.to =
            toIt->second;


        connection.synCount =
            binary.syn_count;


        connection.ntType =
            binary.nt_type;


        std::memcpy(
            connection.neuropil,
            binary.neuropil,
            sizeof(connection.neuropil)
        );


        connection.neuropil[
            sizeof(connection.neuropil) - 1
        ] = '\0';

// ----------------------------------------------------
// ВЕС СВЯЗИ
// ----------------------------------------------------

float weight =
    std::log1p(
        static_cast<float>(binary.syn_count)
    ) / 5.0f;


// Ограничиваем максимальный вес
if (weight > 1.0f)
{
    weight = 1.0f;
}


// Нейромедиатор:
//
// 1 = ACH  → возбуждение
// 2 = GABA → торможение
// 3 = GLUT → торможение
//
// Это пока упрощённая модель.

if (
    binary.nt_type == 2 ||
    binary.nt_type == 3
)
{
    weight = -weight;
}


connection.weight = weight;
        uint32_t connectionIndex =
            static_cast<uint32_t>(
                connections.size()
            );


        connections.push_back(
            connection
        );


        // Добавляем связь в индекс исходящих связей
        outgoingConnections[
            connection.from
        ].push_back(
            connectionIndex
        );


        count++;


        if (
            count % 1000000 == 0
        )
        {
            std::cout
                << "Загружено: "
                << count
                << " связей\n";
        }
    }


    input.close();


    std::cout
        << "\nВсего загружено связей: "
        << count
        << "\n";


    if (skipped > 0)
    {
        std::cout
            << "Пропущено: "
            << skipped
            << "\n";
    }


    std::cout
        << "Размер записи: "
        << sizeof(BinaryConnection)
        << " байт\n";


    return true;
}


// ============================================================
// ЗАГРУЗКА АННОТАЦИЙ
// ============================================================

void FlyBrain::loadNeuronAnnotations(
    const std::string& filename
)
{
    std::ifstream input(
        filename
    );


    if (!input.is_open())
    {
        std::cout
            << "Не удалось открыть файл аннотаций\n";

        return;
    }


    std::string line;


    // Пропускаем заголовок
    std::getline(
        input,
        line
    );


    uint64_t loaded = 0;


    while (
        std::getline(
            input,
            line
        )
    )
    {
        if (line.empty())
        {
            continue;
        }


        std::stringstream ss(line);


        std::string root;
        std::string type;
        std::string family;
        std::string subsystem;
        std::string category;
        std::string side;


        std::getline(
            ss,
            root,
            ','
        );


        std::getline(
            ss,
            type,
            ','
        );


        std::getline(
            ss,
            family,
            ','
        );


        std::getline(
            ss,
            subsystem,
            ','
        );


        std::getline(
            ss,
            category,
            ','
        );


        std::getline(
            ss,
            side,
            ','
        );


        if (root.empty())
        {
            continue;
        }


        try
        {
            NeuronId id =
                std::stoull(root);


            auto it =
                idToIndex.find(id);


            if (
                it == idToIndex.end()
            )
            {
                continue;
            }


            uint32_t index =
                it->second;


            neuronInfo[index].type =
                type;


            neuronInfo[index].family =
                family;


            neuronInfo[index].subsystem =
                subsystem;


            neuronInfo[index].category =
                category;


            neuronInfo[index].side =
                side;


            loaded++;
        }
        catch (...)
        {
            continue;
        }
    }


    input.close();


    std::cout
        << "Аннотаций загружено: "
        << loaded
        << "\n";
}


// ============================================================
// ТИП НЕЙРОНА
// ============================================================

std::string FlyBrain::getNeuronType(
    NeuronId id
) const
{
    auto it =
        idToIndex.find(id);


    if (
        it == idToIndex.end()
    )
    {
        return "Unknown";
    }


    uint32_t index =
        it->second;


    if (
        index >= neuronInfo.size()
    )
    {
        return "Unknown";
    }


    if (
        neuronInfo[index].type.empty()
    )
    {
        return "Unknown";
    }


    return neuronInfo[index].type;
}
std::vector<NeuronId> FlyBrain::getNeuronsByType(
    const std::string& type
) const
{
    std::vector<NeuronId> result;

    for (size_t i = 0; i < neuronInfo.size(); i++)
    {
        if (neuronInfo[i].type == type)
        {
            result.push_back(
                neuronIds[i]
            );
        }
    }

    return result;
}

void FlyBrain::printNeuronTypes() const
{
    std::unordered_set<std::string> types;

    for (const auto& info : neuronInfo)
    {
        if (!info.type.empty())
        {
            types.insert(info.type);
        }
    }

    std::cout << "\n============================\n";
    std::cout << "ТИПЫ НЕЙРОНОВ\n";
    std::cout << "============================\n";

    for (const auto& type : types)
    {
        std::cout << type << "\n";
    }

    std::cout
        << "\nВсего типов: "
        << types.size()
        << "\n";
}
// ============================================================
// ПЕРЕВОД NEUROPIL -> РУССКОЕ НАЗВАНИЕ
// ============================================================

std::string FlyBrain::getNeuropilName(
    const char* neuropil
) const
{
    if (
        neuropil == nullptr ||
        neuropil[0] == '\0'
    )
    {
        return "Неизвестный отдел";
    }


    std::string value(
        neuropil
    );


    // Медулла
    if (
        value == "ME" ||
        value.rfind("ME_", 0) == 0
    )
    {
        return "Медулла";
    }


    // Лобула
    if (
        value == "LO" ||
        value.rfind("LO_", 0) == 0
    )
    {
        return "Лобула";
    }


    // Лобулярная пластинка
    if (
        value == "LOP" ||
        value.rfind("LOP_", 0) == 0
    )
    {
        return "Лобулярная пластинка";
    }


    // Передняя оптическая ямка
    if (
        value == "AME" ||
        value.rfind("AME_", 0) == 0
    )
    {
        return "Передняя медулла";
    }


    if (value == "PB")
    {
        return "Протопонтическое тело";
    }


    if (value == "EB")
    {
        return "Эллипсоидное тело";
    }


    if (value == "FB")
    {
        return "Веерообразное тело";
    }


    if (value == "MB")
    {
        return "Грибовидное тело";
    }


    if (value == "BU")
    {
        return "Булла";
    }


    return "Другой отдел";
}


// ============================================================
// ПОДАЧА СИГНАЛА
// ============================================================

void FlyBrain::setInput(
    NeuronId id,
    float value
)
{
    auto it =
        idToIndex.find(id);


    if (
        it == idToIndex.end()
    )
    {
        std::cout
            << "Нейрон не найден: "
            << id
            << "\n";

        return;
    }


    activation[
        it->second
    ] = std::clamp(
        value,
        0.0f,
        1.0f
    );
}


// ============================================================
// ОДИН ШАГ РАСПРОСТРАНЕНИЯ
// ============================================================

void FlyBrain::update()
{
    std::vector<float> nextActivation(
        activation.size(),
        0.0f
    );

    std::vector<float> strongestInput(
        activation.size(),
        0.0f
    );

    std::fill(
        lastSource.begin(),
        lastSource.end(),
        -1
    );

    // Идём только по активным нейронам
    for (
        uint32_t source = 0;
        source < activation.size();
        source++
    )
    {
        float sourceSignal =
            activation[source];

        if (sourceSignal <= 0.001f)
        {
            continue;
        }

        const auto& outgoing =
            outgoingConnections[source];

        for (
            uint32_t connectionIndex :
            outgoing
        )
        {
            const Connection& connection =
                connections[connectionIndex];

            float transmitted =
                sourceSignal *
                connection.weight;

            nextActivation[
                connection.to
            ] += transmitted;

            if (
                std::abs(transmitted) >
                std::abs(
                    strongestInput[
                        connection.to
                    ]
                )
            )
            {
                strongestInput[
                    connection.to
                ] = transmitted;

                lastSource[
                    connection.to
                ] =
                    static_cast<int32_t>(
                        source
                    );
            }
        }
    }

    // Нормализуем активность
    for (
        size_t i = 0;
        i < nextActivation.size();
        i++
    )
    {
        float x =
            nextActivation[i];

        // Если торможение сильнее возбуждения
        if (x <= 0.0f)
        {
            activation[i] = 0.0f;
        }
        else
        {
            // Мягкое насыщение
            activation[i] =
                x / (1.0f + x);
        }
    }
}

// ============================================================
// АКТИВНОСТЬ НЕЙРОНА
// ============================================================

float FlyBrain::getActivation(
    NeuronId id
) const
{
    auto it =
        idToIndex.find(id);


    if (
        it == idToIndex.end()
    )
    {
        return 0.0f;
    }


    return activation[
        it->second
    ];
}


// ============================================================
// КОЛИЧЕСТВО НЕЙРОНОВ
// ============================================================

int FlyBrain::getNeuronCount() const
{
    return static_cast<int>(
        neuronIds.size()
    );
}


// ============================================================
// ROOT ID ПО ИНДЕКСУ
// ============================================================

NeuronId FlyBrain::getNeuronId(
    int index
) const
{
    if (
        index < 0 ||
        index >=
        static_cast<int>(
            neuronIds.size()
        )
    )
    {
        return 0;
    }


    return neuronIds[index];
}


// ============================================================
// АКТИВНЫЕ НЕЙРОНЫ
// ============================================================

void FlyBrain::printActiveNeurons() const
{
    std::cout
        << "\nАктивные нейроны:\n";


    int count = 0;


    for (
        size_t i = 0;
        i < activation.size();
        i++
    )
    {
        if (
            activation[i] <= 0.001f
        )
        {
            continue;
        }


        std::cout
            << "  "
            << getNeuronType(
                neuronIds[i]
            )
            << " | "
            << neuronIds[i]
            << " | activity = "
            << activation[i]
            << "\n";


        count++;


        if (count >= 30)
        {
            break;
        }
    }


    if (count == 0)
    {
        std::cout
            << "  нет активных нейронов\n";
    }
}


// ============================================================
// АКТИВНЫЕ СВЯЗИ
// ============================================================

void FlyBrain::printActiveConnections() const
{
    std::cout
        << "\nАктивные связи:\n";


    int count = 0;


    for (
        size_t source = 0;
        source < activation.size();
        source++
    )
    {
        if (
            activation[source] <= 0.001f
        )
        {
            continue;
        }


        for (
            uint32_t connectionIndex :
            outgoingConnections[source]
        )
        {
            const Connection& connection =
                connections[connectionIndex];


            float signal =
                activation[source] *
                connection.weight;


            if (
                std::abs(signal) < 0.001f
            )
            {
                continue;
            }


            std::cout
                << "  "
                << getNeuronType(
                    neuronIds[source]
                )
                << " -> "
                << getNeuronType(
                    neuronIds[
                        connection.to
                    ]
                )
                << " | signal = "
                << signal
                << " | отдел: "
                << getNeuropilName(
                    connection.neuropil
                )
                << "\n";


            count++;


            if (count >= 30)
            {
                return;
            }
        }
    }
}


// ============================================================
// ПУТЬ НАЗАД ПО СИЛЬНЕЙШИМ ИСТОЧНИКАМ
// ============================================================

void FlyBrain::printSignalPath(
    NeuronId startId
) const
{
    auto it =
        idToIndex.find(startId);


    if (
        it == idToIndex.end()
    )
    {
        return;
    }


    int current =
        static_cast<int>(
            it->second
        );


    std::cout
        << "\n============================\n";

    std::cout
        << "SIGNAL PATH\n";

    std::cout
        << "============================\n";


    for (
        int step = 0;
        step < 20;
        step++
    )
    {
        if (
            current < 0 ||
            current >=
            static_cast<int>(
                neuronIds.size()
            )
        )
        {
            break;
        }


        std::cout
            << getNeuronType(
                neuronIds[current]
            )
            << " | "
            << neuronIds[current]
            << " | activity = "
            << activation[current]
            << "\n";


        int source =
            lastSource[current];


        if (
            source < 0
        )
        {
            break;
        }


        current = source;
    }
}


// ============================================================
// ПОЛНАЯ ТРАССИРОВКА СИГНАЛА
// ============================================================

void FlyBrain::traceForward(
    NeuronId startId,
    int steps
) const
{
    auto startIt =
        idToIndex.find(startId);

    if (startIt == idToIndex.end())
    {
        std::cout
            << "Стартовый нейрон не найден\n";

        return;
    }

    struct SignalNode
    {
        uint32_t neuron;
        float signal;

        float strongestSignal;
        uint32_t strongestConnection;
    };

    std::vector<SignalNode> current;

    current.push_back(
        {
            startIt->second,
            1.0f,
            0.0f,
            0
        }
    );

    std::vector<bool> visited(
        neuronIds.size(),
        false
    );

    visited[
        startIt->second
    ] = true;

    const int MAX_NEURONS = 30;

    std::cout
        << "\n============================\n"
        << "NORMALIZED SIGNAL TRACE\n"
        << "============================\n";

    for (
        int step = 1;
        step <= steps;
        step++
    )
    {
        std::cout
            << "\nSTEP "
            << step
            << "\n"
            << "----------------------------\n";

        std::unordered_map<
            uint32_t,
            SignalNode
        > nextMap;

        std::unordered_set<
            std::string
        > departments;

        // ================================================
        // 1. РАСПРОСТРАНЯЕМ СИГНАЛ
        // ================================================

        for (
            const SignalNode& node :
            current
        )
        {
            if (node.signal <= 0.001f)
            {
                continue;
            }

            const auto& outgoing =
                outgoingConnections[
                    node.neuron
                ];

            for (
                uint32_t connectionIndex :
                outgoing
            )
            {
                const Connection& connection =
                    connections[
                        connectionIndex
                    ];

                uint32_t target =
                    connection.to;

                if (visited[target])
                {
                    continue;
                }

                float transmitted =
                    node.signal *
                    connection.weight;

                if (
                    std::abs(transmitted)
                    < 0.001f
                )
                {
                    continue;
                }

                auto it =
                    nextMap.find(target);

                if (it == nextMap.end())
                {
                    SignalNode newNode{};

                    newNode.neuron =
                        target;

                    newNode.signal =
                        transmitted;

                    newNode.strongestSignal =
                        transmitted;

                    newNode.strongestConnection =
                        connectionIndex;

                    nextMap[target] =
                        newNode;
                }
                else
                {
                    it->second.signal +=
                        transmitted;

                    if (
                        std::abs(transmitted)
                        >
                        std::abs(
                            it->second.strongestSignal
                        )
                    )
                    {
                        it->second.strongestSignal =
                            transmitted;

                        it->second.strongestConnection =
                            connectionIndex;
                    }
                }

                departments.insert(
                    getNeuropilName(
                        connection.neuropil
                    )
                );
            }
        }

        if (nextMap.empty())
        {
            std::cout
                << "\nСигнал дальше не распространяется.\n";

            break;
        }

        // ================================================
        // 2. НОРМАЛИЗАЦИЯ
        // ================================================

        for (
            auto& [id, node] :
            nextMap
        )
        {
            float x =
                node.signal;

            if (x <= 0.0f)
            {
                node.signal = 0.0f;
            }
            else
            {
                node.signal =
                    x / (1.0f + x);
            }
        }

        // ================================================
        // 3. MAP → VECTOR
        // ================================================

        std::vector<SignalNode> next;

        for (
            const auto& [id, node] :
            nextMap
        )
        {
            if (node.signal > 0.001f)
            {
                next.push_back(node);
            }
        }

        // ================================================
        // 4. СОРТИРОВКА
        // ================================================

        std::sort(
            next.begin(),
            next.end(),
            [](
                const SignalNode& a,
                const SignalNode& b
            )
            {
                return a.signal > b.signal;
            }
        );

        // ================================================
        // 5. ВЫВОД
        // ================================================

        int shown =
            std::min(
                MAX_NEURONS,
                static_cast<int>(
                    next.size()
                )
            );

        for (
            int i = 0;
            i < shown;
            i++
        )
        {
            const SignalNode& node =
                next[i];

            const Connection& connection =
                connections[
                    node.strongestConnection
                ];

            std::cout
                << "  └──> "
                << getNeuronType(
                    neuronIds[node.neuron]
                )
                << " | "
                << neuronIds[node.neuron]
                << " | signal = "
                << node.signal
                << " | ";

            if (node.strongestSignal > 0)
            {
                std::cout
                    << "возбуждение";
            }
            else
            {
                std::cout
                    << "торможение";
            }

            std::cout
                << " | отдел: "
                << getNeuropilName(
                    connection.neuropil
                )
                << "\n";
        }

        if (next.size() > MAX_NEURONS)
        {
            std::cout
                << "\n  ... ещё "
                << next.size() - MAX_NEURONS
                << " нейронов\n";
        }

        // ================================================
        // 6. ОТДЕЛЫ
        // ================================================

        std::cout
            << "\nЗадействованные отделы:\n";

        for (
            const std::string& department :
            departments
        )
        {
            std::cout
                << "  • "
                << department
                << "\n";
        }

        // ================================================
        // 7. ОСТАВЛЯЕМ САМЫЕ СИЛЬНЫЕ
        // ================================================

        if (next.size() > MAX_NEURONS)
        {
            next.resize(MAX_NEURONS);
        }

        for (
            const SignalNode& node :
            next
        )
        {
            visited[node.neuron] = true;
        }

        current =
            std::move(next);
    }
}void FlyBrain::resetActivations()
{
    std::fill(
        activation.begin(),
        activation.end(),
        0.0f
    );

    std::fill(
        lastSource.begin(),
        lastSource.end(),
        -1
    );
}