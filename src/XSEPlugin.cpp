#include "Hooks.h"
#include "Papyrus.h"
#include "ReplacerManager.h"

using namespace PAR;

void InitializeLog([[maybe_unused]] spdlog::level::level_enum a_level = spdlog::level::info)
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		util::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= std::format("{}.log"sv, Plugin::NAME);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	const auto level = a_level;

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
	log->set_level(level);
	log->flush_on(spdlog::level::info);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] [%s:%#] %v");
}


void Listener(SKSE::MessagingInterface::Message* message) noexcept
{
	if (message->type == SKSE::MessagingInterface::kDataLoaded) {
		ReplacerManager::Init();
	}
}

SKSEPluginInfo(
	.Version = Plugin::VERSION,
	.Name = Plugin::NAME,
	.Author = ""sv,
	.SupportEmail = ""sv,
	.StructCompatibility = SKSE::StructCompatibility::Independent,
	.RuntimeCompatibility = SKSE::VersionIndependence::AddressLibrary,
	.MinimumSKSEVersion = REL::Version{ 0, 0, 0, 0 }
)

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();
	logger::info("Loaded plugin {} {}", Plugin::NAME, Plugin::VERSION.string());
	SKSE::Init(a_skse);

	Hooks::Install();

	if (const auto messaging{ SKSE::GetMessagingInterface() }; !messaging->RegisterListener(Listener))
		return false;

	const auto papyrus = SKSE::GetPapyrusInterface();
	papyrus->Register(Papyrus::RegisterFunctions);

	return true;
}