#pragma once
#include <memory>
namespace running
{
    /**
     * @brief Интерфейс запускаемого объекта
     *
     */
    class IRunnable
    {
    public:
        virtual ~IRunnable(){};
        /**
         * @brief Основной метод потока
         *
         * @param terminate Переменная для остановки потока извне
         */
        virtual void run(std::atomic_bool &terminate) = 0;
    };

    using RunnablePtr = std::unique_ptr<IRunnable>;
    /**
     * @brief Класс автоматически запускающий запускаемый объект в отдельном потоке
     *
     * Поток останавливается только при вызове метода terminate() или при уничтожении объекта (вызове
     * деструктора), при выходе из основного цикла потока по другой причине поток перезапускается.
     */
    class AutoThread
    {
    public:
        /**
         * @brief Конструктор с параметрами
         *
         * @param runnable_object RVALUE указатель на запускаемый объект
         * @param core_id Номер ядра процессора для привязки потока, -1 нет привязкиы
         */
        AutoThread(RunnablePtr &&runnable_object);
        AutoThread(RunnablePtr &&runnable_object, int core_id);
        /**
         * @brief Конструктор с параметрами
         *
         * @param runnable_object Указатель на запускаемый объект
         * @param core_id Номер ядра процессора для привязки потока, -1 нет привязкиы
         */
        AutoThread(IRunnable *runnable_object);
        AutoThread(IRunnable *runnable_object, int core_id);
        /**
         * @brief Деструктор
         *
         */
        ~AutoThread();
        /**
         * @brief Проверка остановлен поток или нет
         *
         * @return true Поток остановлен
         * @return false Поток запущен
         */
        bool terminated() const;
        /**
         * @brief Остановка протока
         *
         */
        void terminate();
        /**
         * @brief Получить идентификатор ядра процессора, к которому привязан поток
         *
         * @return int Идентификатор ядра процессора, если -1, то поток не привязан к ядру
         */
        int getCoreId() const;
        AutoThread(const AutoThread &) = delete;
        AutoThread(AutoThread &&) = delete;
        AutoThread &operator=(const AutoThread &) = delete;
        AutoThread &operator=(AutoThread &&) = delete;

    private:
        // Скрытая реализация должна быть объявлена последней
        class Impl;
        std::unique_ptr<Impl> pimpl_;
    };
} // namespace running