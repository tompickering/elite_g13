function is_in(t, val)
    for k, v in ipairs(t) do
        if v == val then
            return true
        end
    end

    return false
end

stars_scoopable = {'O', 'B', 'A', 'F', 'G', 'K', 'M'}
stars_danger = {'D', 'DA', 'DB', 'DC', 'DO', 'DQ', 'DX'}

row = 0
ship = ''
jumping = false
jumps_remaining = 0

function reset(t)
    jumping = false
    row = 0
    reset_lcd()
end

function LoadGame(t)
    ship = t['Ship']
end

function ShipyardSwap(t)
    ship = t['ShipType']
end

function FSDTarget(t)
    jumps_remaining = t['RemainingJumpsInRoute']
end

function StartJump(t)
    if t['JumpType'] == 'Hyperspace' then
        jumping = true

        star_system = t['StarSystem']
        star_class = t['StarClass']

        jumps_remaining_str = tostring(jumps_remaining)

        if is_in(stars_danger, star_class) then
            set_color(0xff, 0x00, 0x00)
        elseif is_in(stars_scoopable, star_class) then
            set_color(0x00, 0xff, 0x00)
        else
            set_color(0x00, 0x00, 0xff)
        end

        clear_lcd()
        draw_string(60, 2, "JUMPING");
        draw_string(4, 10, star_system)
        draw_string(4, 20, star_class)
        draw_string(4, 30, 'JUMPS: ', jumps_remaining_str)
    end
end

function FSSSignalDiscovered(t)
    if jumping then
        jumping = false
        row = 0
        reset_lcd()
    end

    if t['IsStation'] then
        signal_type = t['SignalType']
        if signal_type == 'FleetCarrier' then
            -- I generally don't care, but useful to know there's one there
            set_color(0xff, 0x00, 0xff)
        else
            draw_string(4, 2 + 10*row, 'STATION: ' .. signal_type)
            row = row + 1
        end
    end
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

SupercruiseExit = reset
SupercruiseDestinationDrop = reset
Docked = reset
DockingCancelled = reset
DockingTimeout = reset
