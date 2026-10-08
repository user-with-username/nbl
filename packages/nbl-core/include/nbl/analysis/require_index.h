#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

namespace nbl::utils {
class SourceProvider;
}

namespace nbl::analysis {

/// Обратный индекс require-ов: для каждого модуля хранит множество модулей,
/// которые его require-ят, и множество модулей, которые require-ит он сам.
///
/// Используется LSP-сервером, чтобы понять, является ли файл entrypoint'ом
/// (его никто не требует => обязан иметь `function tick()`).
class RequireIndex {
public:
  /// Полное сканирование workspace. Один раз при старте сервера.
  void rebuild(const std::string &root);

  /// Обновить запись одного файла (didOpen / didChange / watcher).
  void update(const std::string &path,
              const nbl::utils::SourceProvider &sources);

  /// Убрать запись (didClose, watcher-Delete).
  void remove(const std::string &path);

  /// Входит ли файл в проиндексированный workspace.
  bool contains(const std::string &path) const;

  /// Есть ли у файла хотя бы один require со стороны другого модуля.
  bool has_requirers(const std::string &path) const;

private:
  void record(const std::string &path, const std::vector<std::string> &targets);

  std::map<std::string, std::set<std::string>> requirers_;
  std::map<std::string, std::set<std::string>> requires_;
};

} // namespace nbl::analysis