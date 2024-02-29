# Project

## Описание

Проект "Diplom" представляет собой приложение для эффективного поиска в файлах, которые очень редко изменяются. Это приложение идеально подходит для сценариев, где необходимо быстро искать данные в большом количестве статичных файлов. Первый запуск приложения занимает некоторое время, так как происходит индексация файлов, но последующие поисковые запросы выполняются очень быстро, с незначительным изменением времени выполнения при изменении запроса.

## Сборка и Установка

### Подготовка

Для начала необходимо создать папку `build`, которая будет использоваться для сборки проекта:

```
mkdir build
cd build
```
### Конфигурация

Для конфигурации проекта используйте cmake, указав путь установки через параметр -DCMAKE_INSTALL_PREFIX. Если вы хотите выключить сборку тестов, используйте -DBUILD_TESTS=OFF.
```
cmake -DCMAKE_INSTALL_PREFIX=/путь/установки -DBUILD_TESTS=OFF ..
```
### Сборка

Для сборки проекта  в linux выполните следующие команды:

```
cmake ..
make
```
Для сборки проекта  в windows выполните следующие команды:


```
cmake ..
cmake --build .
```


После завершения процесса установки, исполняемый файл приложения будет находиться в директории /путь/установки/bin/.

Для запуска приложения выполните:

```
./Diplom
```
### Запуск Тестов
запуск google test
```
ctest
```
или мои тесты
#### T1 - тесты на ошибки
#### T2 n- тесты потоков, где n количество потоков
```
./DiplomTests
./DiplomTests T1   
./DiplomTests T2 22
```
Использование

Благодарим за использование нашего приложения!


