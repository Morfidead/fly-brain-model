#include <iostream>
#include <fstream>
#include <string>

int main()
{
    std::ifstream file(
        "/Users/mac/Downloads/connections_princeton_no_threshold.csv"
    );

    if (!file.is_open())
    {
        std::cout << "Не удалось открыть файл\n";
        return 1;
    }

    std::string line;

    // Пропускаем заголовок
    std::getline(file, line);

    long long count = 0;

    while (std::getline(file, line))
    {
        if (line.empty())
        {
            continue;
        }

        count++;

        // Каждые 1 000 000 строк сообщаем прогресс
        if (count % 1000000 == 0)
        {
            std::cout
                << "Прочитано: "
                << count
                << " связей\n";
        }
    }

    std::cout << "\nГотово!\n";
    std::cout << "Всего связей: "
              << count
              << "\n";

    return 0;
}