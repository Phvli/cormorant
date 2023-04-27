#include "logger.h"

#include <cstdio>
#include <cstdarg>
#include <cstring>

#include "../../version.h"
#include "../util/file.h"

using namespace core;

Logger::Logger()
{
	const char *filename = DATA_DIRECTORY "../" APP_TITLE ".log";
	
	this->logging_enabled = true;
	this->error_count          = 0;
	this->file            = (void *)fopen(filename, "w");
	
	if (this->file == NULL)
	{
		printf("(!) Failed to write runtime log to %s\n", filename);
		this->error_count++;
		this->logging_enabled = false;
		return;
	}
	
	time(&start_time);
	if (fprintf((FILE *)this->file, "%s (%s)\nLog of the latest runtime,\n%s",
		APP_TITLE, VER_STRING, ctime(&start_time)) < 0)
	{
		printf("(!) Failed to write runtime log to %s\n", filename);
		this->error_count++;
		return;
	}
}

void
Logger::log_header(const char *s)
{
	if (this->logging_enabled)
	{
		fprintf((FILE *)this->file, "\n\n%s", s);
		fflush((FILE *)this->file);
	}
}

void
Logger::log(const char *format, ...)
{
	va_list arguments;
	const char *s;
	int r;

	if (!this->logging_enabled || this->file == NULL)
	{
		return;
	}
	
	if (format[0] == '<')
	{
		s = &format[1];
		fputc(' ', (FILE *)this->file);
	}
	else
	{
		s = format;
		clock_t frac    = clock() * 100 / CLOCKS_PER_SEC;
		clock_t runtime = frac / 100;
		r = fprintf((FILE *)this->file, "\n  %01i:%02i:%02i.%02i - ",
			(int)(runtime / 3600), (int)((runtime / 60) % 60), (int)(runtime % 60), (int)(frac % 100));
		
		if (r < 0)
		{
			return;
		}
	}

	if (strstr(s, "(!)") != NULL)
	{
		this->error_count++;
	}
	
	va_start(arguments, format);
	r = vfprintf((FILE *)this->file, s, arguments);
	va_end(arguments);
	
	fflush((FILE *)this->file);
}

Logger::~Logger()
{
	long runtime;
	time_t current_time;
	
	if (this->file != NULL)
	{
		time(&current_time);
		runtime = difftime(current_time, start_time);
		
		fprintf((FILE *)this->file, "\n\n\nTotal runtime: %02i:%02i:%02i\n",
			(int)((runtime / 3600) % 100), (int)((runtime / 60) % 60), (int)(runtime % 60));
		
		if (this->error_count)
		{
			fprintf((FILE *)this->file, "Errors: %i.\n", this->error_count);
		}
		else
		{
			fprintf((FILE *)this->file, "There were no errors.\n");
		}
		
		// fclose((FILE *)this->file);
	}
}
