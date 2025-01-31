#include "guis/GuiVideoScreensaverOptions.h"

#include "components/OptionListComponent.h"
#include "components/SliderComponent.h"
#include "components/SwitchComponent.h"
#include "guis/GuiMsgBox.h"
#include "Settings.h"

GuiVideoScreensaverOptions::GuiVideoScreensaverOptions(Window* window, const char* title) : GuiScreensaverOptions(window, title)
{
	// timeout to swap videos
	auto swap = std::make_shared<SliderComponent>(mWindow, 10.f, 1000.f, 1.f, "s");
	swap->setValue((float)(Settings::getInstance()->getInt("ScreenSaverSwapVideoTimeout") / (1000)));
	addWithLabel(_("SWAP VIDEO AFTER (SECS)"), swap);
	addSaveFunc([swap] {
		int playNextTimeout = (int)Math::round(swap->getValue()) * (1000);
		Settings::getInstance()->setInt("ScreenSaverSwapVideoTimeout", playNextTimeout);
		PowerSaver::updateTimeouts();
	});

	auto stretch_screensaver = std::make_shared<SwitchComponent>(mWindow);
	stretch_screensaver->setState(Settings::getInstance()->getBool("StretchVideoOnScreenSaver"));
	addWithLabel(_("STRETCH VIDEO ON SCREENSAVER"), stretch_screensaver);
	addSaveFunc([stretch_screensaver] { Settings::getInstance()->setBool("StretchVideoOnScreenSaver", stretch_screensaver->getState()); });

#ifdef _OMX_
	auto ss_omx = std::make_shared<SwitchComponent>(mWindow);
	ss_omx->setState(Settings::getInstance()->getBool("ScreenSaverOmxPlayer"));
	addWithLabel(_("USE OMX PLAYER FOR SCREENSAVER"), ss_omx);
	addSaveFunc([ss_omx, this] { Settings::getInstance()->setBool("ScreenSaverOmxPlayer", ss_omx->getState()); });
#endif

	// Render Video Game Name as subtitles
	auto ss_info = std::make_shared< OptionListComponent<std::string> >(mWindow, _("SHOW GAME INFO"), false);
	std::vector<std::string> info_type;
	info_type.push_back(N_("always"));
	info_type.push_back(N_("start & end"));
	info_type.push_back(N_("never"));
	for(auto it = info_type.cbegin(); it != info_type.cend(); it++)
		ss_info->add(_(it->c_str()), *it, Settings::getInstance()->getString("ScreenSaverGameInfo") == *it);
	addWithLabel(_("SHOW GAME INFO ON SCREENSAVER"), ss_info);
	addSaveFunc([ss_info, this] { Settings::getInstance()->setString("ScreenSaverGameInfo", ss_info->getSelected()); });

	auto ss_video_mute = std::make_shared<SwitchComponent>(mWindow);
	ss_video_mute->setState(Settings::getInstance()->getBool("ScreenSaverVideoMute"));
	addWithLabel(_("MUTE SCREENSAVER AUDIO"), ss_video_mute);
	addSaveFunc([ss_video_mute] { Settings::getInstance()->setBool("ScreenSaverVideoMute", ss_video_mute->getState()); });


	auto ss_vlc_resolution = std::make_shared< OptionListComponent<std::string> >(mWindow, _("VIDEO RESOLUTION"), false);
	std::vector<std::string> vlc_res;
	vlc_res.push_back(N_("original")); // renders at original video resolution, stretched to fit screen
	vlc_res.push_back(N_("low")); // 25% of screen resolution
	vlc_res.push_back(N_("medium")); // 50% of screen resolution
	vlc_res.push_back(N_("high")); // 75% of screen resolution
	vlc_res.push_back(N_("max")); // full screen resolution
	for(auto it = vlc_res.cbegin(); it != vlc_res.cend(); it++)
		ss_vlc_resolution->add(_(it->c_str()), *it, Settings::getInstance()->getString("VlcScreenSaverResolution") == *it);
	addWithLabel(_("VLC: SCREENSAVER VIDEO RESOLUTION"), ss_vlc_resolution);
	addSaveFunc([ss_vlc_resolution, this] { Settings::getInstance()->setString("VlcScreenSaverResolution", ss_vlc_resolution->getSelected()); });

#ifdef _OMX_
	ComponentListRow row;

	// Set subtitle position
	auto ss_omx_subs_align = std::make_shared< OptionListComponent<std::string> >(mWindow, _("GAME INFO ALIGNMENT"), false);
	std::vector<std::string> align_mode;
	align_mode.push_back(N_("left"));
	align_mode.push_back(N_("center"));
	for(auto it = align_mode.cbegin(); it != align_mode.cend(); it++)
		ss_omx_subs_align->add(_(it->c_str()), *it, Settings::getInstance()->getString("SubtitleAlignment") == *it);
	addWithLabel(_("OMX: GAME INFO ALIGNMENT"), ss_omx_subs_align);
	addSaveFunc([ss_omx_subs_align, this] { Settings::getInstance()->setString("SubtitleAlignment", ss_omx_subs_align->getSelected()); });

	// Set font size
	auto ss_omx_font_size = std::make_shared<SliderComponent>(mWindow, 1.f, 64.f, 1.f, "h");
	ss_omx_font_size->setValue((float)(Settings::getInstance()->getInt("SubtitleSize")));
	addWithLabel(_("OMX: GAME INFO FONT SIZE"), ss_omx_font_size);
	addSaveFunc([ss_omx_font_size] {
		int subSize = (int)Math::round(ss_omx_font_size->getValue());
		Settings::getInstance()->setInt("SubtitleSize", subSize);
	});

	// Define subtitle font
	auto ss_omx_font_file = std::make_shared<TextComponent>(mWindow, "", Font::get(FONT_SIZE_SMALL), 0x777777FF);
	addEditableTextComponent(row, _("OMX: PATH TO FONT FILE"), ss_omx_font_file, Settings::getInstance()->getString("SubtitleFont"));
	addSaveFunc([ss_omx_font_file] {
		Settings::getInstance()->setString("SubtitleFont", ss_omx_font_file->getValue());
	});

	// Define subtitle italic font
	auto ss_omx_italic_font_file = std::make_shared<TextComponent>(mWindow, "", Font::get(FONT_SIZE_SMALL), 0x777777FF);
	addEditableTextComponent(row, _("OMX: PATH TO ITALIC FONT FILE"), ss_omx_italic_font_file, Settings::getInstance()->getString("SubtitleItalicFont"));
	addSaveFunc([ss_omx_italic_font_file] {
		Settings::getInstance()->setString("SubtitleItalicFont", ss_omx_italic_font_file->getValue());
	});
#endif
}

GuiVideoScreensaverOptions::~GuiVideoScreensaverOptions()
{
}

void GuiVideoScreensaverOptions::save()
{
#ifdef _OMX_
	bool startingStatusNotRisky = (Settings::getInstance()->getString("ScreenSaverGameInfo") == "never" || !Settings::getInstance()->getBool("ScreenSaverOmxPlayer"));
#endif
	GuiScreensaverOptions::save();

#ifdef _OMX_
	bool endStatusRisky = (Settings::getInstance()->getString("ScreenSaverGameInfo") != "never" && Settings::getInstance()->getBool("ScreenSaverOmxPlayer"));
	if (startingStatusNotRisky && endStatusRisky) {
		// if before it wasn't risky but now there's a risk of problems, show warning
		mWindow->pushGui(new GuiMsgBox(mWindow,
		_("Using OMX Player and displaying Game Info may result in the video flickering in some TV modes. If that happens, consider:\n\n- Disabling the \"Show Game Info\" option;\n- Disabling \"Overscan\" on the Pi configuration menu might help:\nRetroPie > Raspi-Config > Advanced Options > Overscan > \"No\".\n- Disabling the use of OMX Player for the screensaver."),
			_("GOT IT!") , [] { return; }));
	}
#endif
}
