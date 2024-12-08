function DockingGranted(t)
    print("Docking granted on pad " .. t['LandingPad'])
    set_color(0x00, 0xff, 0x00)
    draw_string_scaled(36, 2, "GRANTED", 2);
end
