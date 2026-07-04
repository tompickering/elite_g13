ANACONDA = 'anaconda'
ASPX = 'asp'
CORVETTE = 'federation_corvette'
DBX = 'diamondbackxl'
KRAIT = 'Krait_MkII'
MANDALAY = 'mandalay'
PYTHON_II = 'Python_NX'
SIDEWINDER = 'sidewinder'
T8 = 'type8'
T10 = 'type9_military'
VULTURE = 'vulture'

icons = {}

merits = 0

function init()
    icons['Federation'] = define_icon({4064,12568,16644,33666,36834,66433,65793,78113,77825,65553,65593,65661,32826,32786,16388,12312,4064})
    icons['Empire'] = define_icon({0, 0, 1966095,986142,495164,247928,124144,61920,32704,16256,7936,7936,3584,1024, 0, 0, 0})
    icons['Alliance'] = define_icon({4064,12312,18372,39090,41610,74377,83653,83653,85733,85349,89013,79817,45050,38962,18372,12312,4064})

    --draw_icon(icons['Federation'], 120, 4)

    --draw_string(90, 34, 'LIMPETS: ' .. zero_pad(limpets, 3))
end

function zero_pad(n, l)
    n = tostring(n)

    while #n < l do
        n = '0' .. n
    end

    return n
end

function is_in(t, val)
    for k, v in ipairs(t) do
        if v == val then
            return true
        end
    end

    return false
end

mode = 'normal'

stars_scoopable = {'O', 'B', 'A', 'F', 'G', 'K', 'M'}
stars_danger = {'D', 'DA', 'DB', 'DC', 'DO', 'DQ', 'DX', 'N'}

row = 0
rows = {}

ship = ''
jumps_remaining = 0
limpetcontroller = true
limpets = 0
exobio_planets = {}

function clear()
    row = 0
    clear_lcd()
end

function reset()
    row = 0
    reset_lcd()
end

function set_ship(s)
    ship = s
    print('Using ship: ' .. ship)
end

function set_mode(m)
    mode = m
    print('Setting mode: ' .. mode)

    clear()

    if mode == 'normal' then
        set_color(0x00, 0x00, 0xff)
    end

    if mode == 'combat' then
        set_color(0x00, 0xff, 0xff)
    end

    if mode == 'exobio' then
        update_exobio()
    end

    if mode == 'merits' then
        set_color(0xff, 0xff, 0x00)
        ShowMerits()
    end
end

function Mode_1() set_mode('normal') end
function Mode_2() set_mode('combat') end
function Mode_3() set_mode('exobio') end
function Mode_4() set_mode('merits') end


-- EVENTS --


function LoadGame(t)
    set_ship(t['Ship'])
end

function Loadout(t)
    mod = t['Modules']

    limpetcontroller = false

    for i, v in ipairs(mod) do
        if string.find(v['Item'], "dronecontrol") then
            limpetcontroller = true
            return
        end
    end
end

function Cargo(t)
    limpets = 0

    inv = t['Inventory']

    if inv == nil then return end

    for i, v in ipairs(inv) do
        if v['Name'] == 'drones' then
            limpets = v['Count']
        end
    end
end

function BuyDrones(t)
    limpets = limpets + t['Count']
end

function SellDrones(t)
    limpets = limpets - t['Count']
end

function LaunchDrone(t)
    limpets = limpets - 1
end

function NavRoute(t)
    if limpetcontroller then
        draw_string(90, 34, 'LIMPETS: ' .. zero_pad(limpets, 3))
    end
end

function ShipyardSwap(t)
    set_ship(t['ShipType'])
end

function FSDTarget(t)
    jumps_remaining = t['RemainingJumpsInRoute']
end

function StartJump(t)
    if t['JumpType'] == 'Hyperspace' then
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
        draw_string(60, 1, "JUMPING");
        draw_string(4, 10, star_system)
        draw_string(4, 20, star_class)
        draw_string(4, 30, 'JUMPS: ')
        draw_string_scaled(46, 24, jumps_remaining_str, 2)

        exobio_planets = {}
    end
end

function FSDJump(t)
    pp_state = t['PowerplayState']
    pp_power = t['ControllingPower']
    pp_powers = t['Powers']

    c = false

    if pp_power ~= nil then
        clear()
        c = true
        draw_string(4, 1, pp_power);
    end

    if pp_state ~= nil then
        if not c then
            clear()
            c = true
        end
        draw_string(4, 11, pp_state);
    end

    if pp_powers ~= nil then
        if not c then
            clear()
            c = true
        end
        print('Powers:')
        for i, v in ipairs(pp_powers) do
            print(i)
            print(v)
            if v == 'Yuri Grom' then
                draw_string(4, 21, 'YG');
            end
        end
    end
end

function _FSSSignalDiscovered(t)
    to_draw = nil

    if t['IsStation'] and row < 4 then
        signal_type = t['SignalType']
        if signal_type ~= 'FleetCarrier' then
            signal_name = t['SignalName']

            print('SIGNAL: ' .. signal_name)
            for i, v in ipairs(rows) do
                print('IN ROWS: ' .. v)
                if i > row then
                    signal_name = nil
                    print('BREAKING')
                    break
                end

                if v == signal_name then
                    signal_name = nil
                    print('BREAKING')
                    break
                end

                print('NOT EQUAL: ' .. v .. '/' .. signal_name)
            end

            if signal_name ~= nil then
                rows[row] = signal_name
                print(signal_name .. ':' .. signal_type)
                to_draw = 'S: ' .. signal_name
            end
        end
    end

    if to_draw ~= nil then
        if row == 0 then
            reset()
        end

        draw_string(4, 2 + 10*row, to_draw)
        row = row + 1
    end
end

function DockingGranted(t)
    clear()
    set_color(0x00, 0xff, 0x00)
    draw_string_scaled(36, 4, "GRANTED", 2);
    landing_pad = tostring(t['LandingPad'])
    draw_string_scaled(60, 20, zero_pad(landing_pad, 2), 3)
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
    elseif allegiance == 'Independent' then
        set_color(0xff, 0xff, 0x00)
    else
        set_color(0x00, 0xff, 0xff)
    end

    allegiance_icon = icons[allegiance]

    if allegiance_icon ~= nil then
        draw_icon(allegiance_icon, 136, 4)
    end

    for k, v in ipairs(services) do
        print(tostring(k) .. " : " .. tostring(v))
    end

    draw_string_scaled(4, 4, t['StationEconomy_Localised'], 2);
end

function ShieldState(t)
    clear()
    if t['ShieldsUp'] then
        set_color(0x00, 0xff, 0xff)
        draw_string_scaled(24, 10, "SHIELDS UP", 2)
    else
        set_color(0xff, 0x00, 0x00)
        draw_string_scaled(12, 10, "SHIELDS DOWN", 2)
    end
end

function update_exobio()
    reset()

    --draw_string_scaled(38, 4, "EXOBIO", 2);
    --draw_string_scaled(60, 20, zero_pad(exobio_planets, 2), 3)
    sorted = {}
    idx = 1
    for k, v in pairs(exobio_planets) do
        sorted[idx] = k
        idx = idx + 1
    end

    if idx == 1 then
        -- No exobio data
        set_color(0x00, 0x00, 0xff)
        return
    end

    set_color(0xff, 0x00, 0xff)

    table.sort(sorted)

    for i, body in ipairs(sorted) do
        n = i - 1
        row = n % 4
        col = n // 4
        print(tostring(row) .. ' ' .. tostring(col) .. ': ' .. body)
        draw_string(2 + col*30, 2 + row*10, body);
    end
end

function Scan(t)
    if t['ScanType'] == 'Detailed' then
        if t['Landable'] and t['AtmosphereType'] ~= "None" then
            high_metal = false

            if t['Composition'] ~= nil then
                for k, v in pairs(t['Composition']) do
                    if k == 'Metal' then
                        if v > 0.28 then
                            high_metal = true
                        end
                    end
                end
            end

            for k, v in pairs(t['AtmosphereComposition']) do
                if    (v['Name'] == 'SulphurDioxide' and high_metal)
                   or (v['Name'] == 'CarbonDioxide' and high_metal)
                    or v['Name'] == 'Ammonia'
                    or v['Name'] == 'Nitrogen'
                    or v['Name'] == 'Oxygen'
                    or v['Name'] == 'Water'
                    or v['Name'] == 'Neon'
                    or v['Name'] == 'Helium' then
                        body = t['BodyName']
                        star = t['StarSystem']
                        exobio_key = string.sub(body, #star+2, #body)
                        exobio_planets[exobio_key] = true
                        update_exobio()
                end
            end
        end
    end
end

function PowerplayMerits(t)
    merits = merits + t['MeritsGained']
end


-- EVENTS END --


function ResetMerits()
    clear()
    ShowMerits()
    merits = 0
    draw_string(4, 10, 'RESET');
end

function ShowMerits()
    clear()
    draw_string(4, 1, 'MERITS: ' .. zero_pad(merits, 6));
end

SupercruiseExit = reset
SupercruiseDestinationDrop = reset
DockingCancelled = reset
DockingTimeout = reset
