/*! \file */ // Copyright 2011-2020 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md for rights.

#include "fs/File.hpp"
#include "sys/Appfs.hpp"
#include "sys/Cli.hpp"
#include "sys/Printer.hpp"

using namespace var;
using namespace sys;

Printer& sys::operator << (Printer& printer, const Cli & a){
	printer.print_open_object(
				printer.verbose_level(),
				a.name().cstring()
				);
	{
		printer.key("publisher", a.publisher());
		printer.print_open_object(printer.verbose_level(), "arguments");
		{
			for(u32 i = 0; i < a.count(); i++){
				printer.key(0, "%s", a.at(i).cstring());
			}
			printer.print_close_object();
		}
		printer.print_close_object();
	}

	return printer;
}


Cli::Cli(
		int argc,
		char * argv[],
		const char * app_git_hash
		) : m_app_git_hash(app_git_hash){
	u16 version;
	if( argc < 0 ){
		argc = 0;
	}
	m_argc = argc;
	m_argv = argv;
	m_is_case_sensitive = true;

	if( argc > 0 ){
		m_path = argv[0];
		m_name = fs::File::name(
					argv[0]
				);
		version = Appfs::get_version(
					m_path
			#if defined __link
					, fs::File::LinkDriver(nullptr)
			#endif
					);


		m_version.format("%d.%d", version >> 8, version & 0xff);
	}
}

void Cli::handle_version() const {
#if !defined __link
	if( is_option("--version") || is_option("-v") ){
		printf("%s version: %s by %s\n", m_name.cstring(), m_version.cstring(), m_publisher.cstring());
		exit(0);
	}

	if( is_option("--version-details") ){
		String details = get_version_details();
		printf("%s\n", details.cstring());
		exit(0);
	}
#endif
}

var::String Cli::get_version_details() const {
	String result;

	if( m_app_git_hash == 0 ){
		result.format("%s (api:%s)", m_version.cstring(), api::ApiInfo::git_hash());
	} else {
		result.format(
					"%s (app:%s, api:%s)",
					m_version.cstring(),
					m_app_git_hash,
					api::ApiInfo::git_hash()
					);
	}

	return result;
}

var::String Cli::to_string() const {
	String result;
	for(u32 i=1; i < count(); i++){
		result << at(i);
		if( i < count()-1){
			result << " ";
		}
	}
	return result;
}


String Cli::at(u16 value) const {
	String arg;
	if( value < m_argc ){
		arg.assign(m_argv[value]);
	}
	return arg;
}

bool Cli::is_option_equivalent_to_argument(
		const String & option,
		const String & argument) const {
	String option_string = option;
	String argument_string = argument;
	if( is_case_senstive() == false ){
		option_string.to_upper();
		argument_string.to_upper();
	}

	return compare_with_prefix(option_string, argument_string);
}

bool Cli::compare_with_prefix(
		const var::String & option,
		const var::String & argument
		) const {
	String with_prefix;
	if( argument.at(0) != '-' ){ return false; }
	if( option == argument ){ return true; }
	with_prefix << "--" << option;
	if( with_prefix == argument ){ return true; }
	if( with_prefix.create_sub_string(
			 String::Position(1)
			 ) == argument ){ return true; }
	return false;
}


bool Cli::is_option_equivalent_to_argument_with_equality(
		const var::String & option,
		const var::String & argument,
		var::String & value) const {

	if( argument.at(0) != '-' ){
		return false;
	}

	Tokenizer tokens(
				argument,
				var::Tokenizer::Delimeters("="),
				var::Tokenizer::IgnoreBetween(""),
				var::Tokenizer::MaximumCount(1));
	if( tokens.count() == 2 ){
		String a = option;
		String b = tokens.at(0);
		if( is_case_senstive() == false ){
			a.to_upper();
			b.to_upper();
		}

		if( compare_with_prefix(a, b) ){
			value = tokens.at(1);
			return true;
		}
	}
	return false;
}


var::String Cli::get_option(
		const String & name,
		Description help
		) const {
	var::String result;
	u32 args;

	if( help.argument().is_empty() == false ){
		m_help_list.push_back(
					String()
					<< name
					<< ": "
					<< help.argument());
	}

	for(args = 1; args < count(); args++){
		if( is_option_equivalent_to_argument(name, at(args)) ){
			if( count() > args+1 ){
				String value = at(args+1);
				if( value.at(0) == '-' ){
					result = "true";
				} else {
					result = at(args+1);
				}
				return result;
			} else {
				return String("true");
			}
		} else if( is_option_equivalent_to_argument_with_equality(
						  name,
						  at(args),
						  result
						  )
					  ){
			return result;
		}
	}
	return String();
}

String Cli::get_option_argument(const char * option) const {
	u16 args;
	for(args = 0; args < m_argc; args++){
		if( is_option_equivalent_to_argument(option, at(args).cstring()) ){
			return at(args+1);
		}
	}
	return String();
}

bool Cli::is_option(const var::String & value) const {
	u16 i;
	for(i=0; i < m_argc; i++){
		if( is_option_equivalent_to_argument(value, at(i).cstring()) ){
			return true;
		}
	}
	return false;
}

int Cli::get_option_value(const char * option) const {
	String arg = get_option_argument(option);
	if( arg.is_empty() ){
		return 0;
	}
	return arg.to_integer();
}

int Cli::get_option_hex_value(const char * option) const {
	int value;
	String arg = get_option_argument(option);
	if( arg.is_empty() ){
		return 0;
	}
	value = 0;
	sscanf(arg.cstring(), "0x%X", &value);
	return value;
}


mcu_pin_t Cli::get_option_pin(const char * option) const {
	mcu_pin_t pio;
	Tokenizer arg(
				option,
				var::Tokenizer::Delimeters(".")
				);

	if( arg.count() == 2 ){
		pio.port = arg.at(0).to_integer();
		pio.pin = arg.at(1).to_integer();
	} else {
		pio.port = 255;
		pio.pin = 255;
	}

	return pio;
}

mcu_pin_t Cli::pin_at(u16 value) const {
	mcu_pin_t pio;
	Tokenizer arg(
				at(value),
				var::Tokenizer::Delimeters(".")
				);

	if( arg.count() == 2 ){
		pio.port = arg.at(0).to_integer();
		pio.pin = arg.at(1).to_integer();
	} else {
		pio.port = 255;
		pio.pin = 255;
	}

	return pio;
}

int Cli::value_at(u16 value) const {
	return at(value).to_integer();
}

bool Cli::handle_uart(hal::UartAttributes & attr) const {
	if( is_option("-uart") ){
		enum hal::Uart::flags o_flags = hal::Uart::flag_set_line_coding;
		attr.set_port(get_option_value("-uart"));

		if( is_option("-freq") ){
			attr.set_freq(get_option_value("-freq"));
		} else {
			attr.set_freq(115200);
		}

		if( is_option("-even") ){
			o_flags |= hal::Uart::flag_is_parity_even;
		} else if( is_option("-odd") ){
			o_flags |= hal::Uart::flag_is_parity_odd;
		}

		if( is_option("-stop1") ){
			o_flags |= hal::Uart::flag_is_stop1;
		} else if( is_option("-stop2") ){
			o_flags |= hal::Uart::flag_is_stop2;
		} else {
			o_flags |= hal::Uart::flag_is_stop1;
		}

		if( is_option("-tx") ){ attr.set_tx(get_option_pin("-tx")); }
		if( is_option("-rx") ){ attr.set_rx(get_option_pin("-rx")); }
		if( is_option("-rts") ){ attr.set_rts(get_option_pin("-rts")); }
		if( is_option("-cts") ){ attr.set_cts(get_option_pin("-cts")); }

		if( is_option("-width") ){
			attr.set_width(get_option_value("-width"));
		} else {
			attr.set_width(8);
		}

		attr.set_flags(o_flags);

		return true;
	}
	return false;
}

bool Cli::handle_i2c(hal::I2CAttr & attr) const {
	if( is_option("-i2c") ){
		enum hal::I2C::flags o_flags = hal::I2C::flag_set_master;
		attr.set_port(get_option_value("-i2c"));

		if( is_option("-freq") ){
			attr.set_freq(get_option_value("-freq"));
		} else {
			attr.set_freq(100000);
		}

		if( is_option("-slave") ){ attr.set_slave_addr(get_option_hex_value("-slave")); }
		if( is_option("-pu") ){ o_flags |= hal::I2C::flag_is_pullup; }


		if( is_option("-sda") ){ attr.set_sda(get_option_pin("-sda")); }
		if( is_option("-scl") ){ attr.set_scl(get_option_pin("-scl")); }

		attr.set_flags(o_flags);

		return true;
	}
	return false;
}


void Cli::show_options() const {
	printf("%s options:\n", name().cstring());
	for(u32 i=0; i < m_help_list.count(); i++){
		printf("- %s\n", m_help_list.at(i).cstring());
	}
}


