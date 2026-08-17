#include <bulbtils/file.h>
#include <bulbtils/string.h>

#include <fstream>

namespace bulbtils::file
{
	result base::load(const settings& load_settings)
	{
		auto& filename = load_settings.filename;
		auto log = load_settings.log;

		if (!std::filesystem::exists(filename))
		{
			if (load_settings.create_if_not_found)
			{
				if (load_settings.warning_callback)
				{
					load_settings.warning_callback(std::format("Error loading '{}' - file not found, writing default data...", filename));
				}

				// copy settings for saving defaults, but use our own logs instead (if enabled)
				settings save_settings = load_settings;
				save_settings.log = false;

				auto result = save(save_settings);
				if (load_settings.error_callback)
				{
					if (result == r_open_file_error)
					{
						load_settings.error_callback(std::format("Failed to open file '{}' for writing default data", filename));
					}
					else if (result == r_write_error)
					{
						load_settings.error_callback(std::format("Failed to write default data to file '{}'", filename));
					}
				}

				// if successful, default file was created - but still return "file not found"
				return (result == r_success) ? r_file_not_found : result;
			}

			if (load_settings.error_callback)
			{
				load_settings.error_callback(std::format("Error loading '{}' - file not found", filename));
			}
			return r_file_not_found;
		}

		std::ifstream config_file(filename);
		if (!config_file)
		{
			if (load_settings.error_callback)
			{
				load_settings.error_callback(std::format("Error loading '{}' - failed to open file", filename));
			}
			return r_open_file_error;
		}

		auto size = std::filesystem::file_size(filename);
		std::string data(size, '\0');
		if (!(config_file.read(&data[0], size)) && !(config_file.eof()))
		{
			if (load_settings.error_callback)
			{
				load_settings.error_callback(std::format("Error loading '{}' - failed to read from file", filename));
			}
			return r_read_error;
		}

		if (!load_to_struct(data, load_settings))
		{
			if (load_settings.error_callback)
			{
				load_settings.error_callback(std::format("Error loading '{}' - failed to parse data from file", filename));
			}
			return r_parse_error;
		}
		return r_success;
	}

	result base::save(const settings& save_settings) const
	{
		auto& filename = save_settings.filename;
		auto log = save_settings.log;

		if (!std::filesystem::exists(filename) && !save_settings.create_if_not_found)
		{
			if (load_settings.error_callback)
			{
				load_settings.error_callback(std::format("Error saving '{}' - file not found", filename));
			}
			return r_file_not_found;
		}

		std::ofstream file(filename);
		if (!file)
		{
			if (load_settings.error_callback)
			{
				load_settings.error_callback(std::format("Error saving '{}' - failed to open file", filename));
			}
			return r_open_file_error;
		}

		if (!(file << save_from_struct(save_settings)))
		{
			if (load_settings.error_callback)
			{
				load_settings.error_callback(std::format("Error saving '{}' - failed to write to file", filename));
			}
			return r_write_error;
		}
		return r_success;
	}

	uintmax_t get_folder_size(const std::filesystem::path& folder)
	{
		uintmax_t size = 0;
		for (auto& entry : std::filesystem::recursive_directory_iterator(folder))
		{
			if (entry.is_regular_file())
			{
				size += entry.file_size();
			}
		}
		return size;
	}

	std::string size_to_string(uintmax_t size_in_bytes, int decimals)
	{
		static const std::vector<std::string> levels { "B", "KiB", "MiB", "GiB", "TiB" };

		double size = size_in_bytes;
		for (int i = 0; i < levels.size(); i++)
		{
			if (size < 1'024.0)
			{
				return std::format("{:.{}f} {}", size, decimals, levels[i]);
			}
			size /= 1'024.0;
		}
		return std::format("{:.{}f} PiB", size, decimals);
	}

	uintmax_t string_to_size(const std::string& string)
	{
		static const std::vector<std::string> levels { "b", "kib", "mib", "gib", "tib", "pib" };

		auto split = bulbtils::string::split_by_whitespace(string);
		auto size = split.size();
		if (size != 2)
		{
			throw std::invalid_argument(std::format("String '{}' is invalid: expected 2 tokens, got {}", string, size));
		}

		double size = std::stod(split[0]);
		if (size < 0.0)
		{
			throw std::invalid_argument(std::format("String '{}' is invalid: size cannot be below 0", string));
		}

		auto level = bulbtils::string::inplace::to_lowercase(split[1]);
		double multiplier = 1.0;
		for (int i = 0; i < levels.size(); i++)
		{
			if (level == levels[i])
			{
				return size * multiplier;
			}
			multiplier *= 1024.0;
		}
		throw std::invalid_argument(std::format("String '{}' is invalid: unrecognized level '{}'", string, level));
	}
}