#pragma once

#include <vector>
#include <cstdint>
#include <unordered_map>
#include <string>

using NeuronId = uint64_t;


// Информация о нейроне
struct NeuronInfo
{
    std::string type;
    std::string family;
    std::string subsystem;
    std::string category;
    std::string side;
};


// Одна запись из flywire_connections.bin
// Размер = 40 байт
struct BinaryConnection
{
    uint64_t pre_root_id;
    uint64_t post_root_id;
    uint16_t syn_count;
    uint8_t nt_type;
    char neuropil[16];
};


// Связь внутри программы
struct Connection
{
    uint32_t from;
    uint32_t to;

    uint16_t synCount;
    uint8_t ntType;

    char neuropil[16];

    float weight;
};
enum class FlyAction
{
    LEFT,
    RIGHT,
    FORWARD,
    NONE
};



class FlyBrain
{
private:

    // Все root_id нейронов
    std::vector<NeuronId> neuronIds;

    // Информация о нейронах
    std::vector<NeuronInfo> neuronInfo;

    // root_id -> внутренний индекс
    std::unordered_map<NeuronId, uint32_t> idToIndex;


    // Все связи
    std::vector<Connection> connections;


    // Индекс исходящих связей.
    //
    // outgoingConnections[123]
    // содержит номера всех связей,
    // которые выходят из нейрона 123.
    std::vector<std::vector<uint32_t>> outgoingConnections;


    // Текущая активность нейронов
    std::vector<float> activation;


    // Какой нейрон был главным источником сигнала
    std::vector<int32_t> lastSource;


public:

    FlyBrain();


    bool loadNeuronIndex(
        const std::string& filename
    );


    bool loadConnections(
        const std::string& filename
    );


    void loadNeuronAnnotations(
        const std::string& filename
    );


    std::string getNeuronType(
        NeuronId id
    ) const;
    std::vector<NeuronId> getNeuronsByType(
    const std::string& type
) const;
    void printNeuronTypes() const;

    std::string getNeuropilName(
        const char* neuropil
    ) const;


    void setInput(
        NeuronId id,
        float value
    );
    void resetActivations();
    void update();


    float getActivation(
        NeuronId id
    ) const;


    int getNeuronCount() const;


    NeuronId getNeuronId(
        int index
    ) const;


    void printActiveNeurons() const;


    void printActiveConnections() const;


    void printSignalPath(
        NeuronId startId
    ) const;


    void traceForward(
        NeuronId startId,
        int steps
    ) const;
};


struct ActionResult
{
    FlyAction action;

    float left;
    float right;
    float forward;
};