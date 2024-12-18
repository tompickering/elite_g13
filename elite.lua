icons = {}

function init()
    icons['Federation'] = define_icon({4064,12568,16644,33666,36834,66433,65793,78113,77825,65553,65593,65661,32826,32786,16388,12312,4064})
    icons['Empire'] = define_icon({0, 0, 1966095,986142,495164,247928,124144,61920,32704,16256,7936,7936,3584,1024, 0, 0, 0})
    icons['Alliance'] = define_icon({4064,12312,18372,39090,41610,74377,83653,83653,85733,85349,89013,79817,45050,38962,18372,12312,4064})

    draw_icon(icons['Federation'], 120, 4)
end

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

function clear()
    jumping = false
    row = 0
    clear_lcd()
end

function reset()
    jumping = false
    row = 0
    reset_lcd()
end

function set_ship(s)
    ship = s
    print('Using ship: ' .. ship)
end

function LoadGame(t)
    set_ship(t['Ship'])
end

function ShipyardSwap(t)
    set_ship(t['ShipType'])
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

        clear()
        draw_string(60, 2, "JUMPING");
        draw_string(4, 10, star_system)
        draw_string(4, 20, star_class)
        draw_string(4, 30, 'JUMPS: ' .. jumps_remaining_str)
    end
end

function FSSSignalDiscovered(t)
    if jumping or row == 0 then
        reset()
    end

    if t['IsStation'] and row < 4 then
        signal_type = t['SignalType']
        if signal_type ~= 'FleetCarrier' then
            signal_name = t['SignalName']
            draw_string(4, 2 + 10*row, 'S: ' .. signal_name)
            row = row + 1
        end
    end
end

function DockingGranted(t)
    clear()
    set_color(0x00, 0xff, 0x00)
    draw_string_scaled(36, 4, "GRANTED", 2);
    landing_pad = tostring(t['LandingPad'])
    if #landing_pad < 2 then
        landing_pad = '0' .. landing_pad
    end
    draw_string_scaled(60, 20, landing_pad, 3)
end

function DockingDenied(t)
    clear()
    set_color(0xff, 0x00, 0x00)
    draw_string_scaled(28, 10, "DENIED", 3)
end

function Docked(t)
    clear()

    allegiance = t['StationAllegiance']
    economies = t['StationEconomies']
    services = t['StationServices']

    if allegiance == 'Federation' then
        set_color(0xff, 0x66, 0x00)
    elseif allegiance == 'Empire' then
        set_color(0x00, 0x00, 0xff)
    elseif allegiance == 'Alliance' then
        set_color(0x00, 0xff, 0x00)
    else
        set_color(0xff, 0xff, 0xff)
    end

    allegiance_icon = icons[allegiance]

    if allegiance_icon ~= nil then
        draw_icon(allegiance_icon, 136, 4)
    end

    for k, v in ipairs(services) do
        print(tostring(k) .. " : " .. tostring(v))
    end
end

SupercruiseExit = reset
SupercruiseDestinationDrop = reset
DockingCancelled = reset
DockingTimeout = reset
