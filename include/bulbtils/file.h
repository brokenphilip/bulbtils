#pragma once

#include <format>
#include <filesystem>

namespace bulbtils::file
{
	// Settings used by file::base when loading and saving files
	struct settings
	{
		// Name of the file being saved, relative to the current directory
		std::string filename = "";

		// Should files be created if they aren't found?
		// If used with load(), a file containing default data will be created.
		bool create_if_not_found = false;

        // Callback which receives warnings for logging purposes
        std::function<void(const std::string&)> warning_callback = {};

        // Callback which receives errors for logging purposes
        std::function<void(const std::string&)> error_callback = {};
	};

	// Enum for results returned by file::base::load/save
	enum result
	{
		r_success,
		r_file_not_found,
		r_open_file_error,
		r_read_error,
		r_write_error,
		r_parse_error,
	};

	// Base file structure, which can save (or load) string data to (or from) a file
	struct base
	{
		// Use this function to convert your data structure to a string, which will be written to a file
		virtual std::string save_from_struct(const settings& save_settings) const = 0;

		// Use this function to convert the string data loaded from a file to your data structure
		// Return false to pass 's_parse_error' to the load() function
		// (if you're okay with partial loads, particularly if you make use of default values, return true)
		virtual bool load_to_struct(const std::string& data, const settings& load_settings) = 0;

		// Load data structure from file, based on load settings. On success, returns 'r_success' - on failure, returns:
		// - 'r_open_file_error' if it failed to open the file for reading (or writing default data)
		// - 'r_write_error' if it failed to write defaults to the file
		// - 'r_read_error' if it failed to read the file
		// - 'r_file_not_found' if the file was not found (default data will be written if create_if_not_found is enabled)
		// - 'r_parse_error' if the data couldn't be parsed (load_to_struct returned false)
		result load(const settings& load_settings);

		// Load data structure from file, based on load settings. On success, returns 'r_success' - on failure, returns:
		// - 'r_open_file_error' if it failed to open the file for writing
		// - 'r_write_error' if it failed to write contents to the file
		// - 'r_file_not_found' if the file was not found and create_if_not_found is disabled
		result save(const settings& save_settings) const;
	};

	// Given a path to a folder, get its size in bytes
	// Can throw OS I/O exceptions
	uintmax_t get_folder_size(const std::filesystem::path& folder);

	// Convert a size in bytes to its string representation
	// Ranges from bytes (B) to petibytes (PiB)
	std::string size_to_string(uintmax_t size_in_bytes, int decimals = 2);

	// Convert a string representation of size to bytes
	// Can throw invalid_argument and out_of_range
	uintmax_t string_to_size(const std::string& string);
}