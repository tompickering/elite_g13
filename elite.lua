function reset(t)
    reset_screen()
end

function DockingGranted(t)
    clear_lcd()
    set_color(0x00, 0xff, 0x00)
    draw_string_scaled(36, 2, "GRANTED", 2);
    landing_pad = tostring(t['LandingPad'])
    if #landing_pad < 2 then
        landing_pad = '0' .. landing_pad
    end
    draw_string_scaled(60, 18, landing_pad, 3)
end

function DockingDenied(t)
    clear_lcd()
    set_color(0xff, 0x00, 0x00)
    draw_string_scaled(28, 10, "DENIED", 3)
end

Docked = reset
DockingCancelled = reset
DockingTimeout = reset
