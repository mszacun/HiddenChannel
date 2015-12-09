import subprocess

def get_paramater_step(current_value):
    return 0.01

def make_simulation(params):
    args = "./symulator --timeForGeneratingHiddenMessages {timeForGeneratingHiddenMessages} --numberOfHiddenMessagesToGenerate {numberOfHiddenMessagesToGenerate} --timeForGeneratingBasicMessages {timeForGeneratingBasicMessages} --hiddenDataSegmentLength {hiddenDataSegmentLength} --channelBandwidth {channelBandwidth} --hiddenDataAppearance {hiddenDataAppearance} --basicDataAppearance {basicDataAppearance} --basicDataLength {basicDataLength} --hiddenMessageSegmentLength {hiddenMessageSegmentLength} --hiddenDataSegmentValue {hiddenDataSegmentValue} --numberOfSymulations {numberOfSymulations} --timeForSimulation {timeForSimulation}".format(**params)

    results = subprocess.check_output(args.split())
    lines = results.split(b'\n')
    hidden_messages_delay_line = lines[-3]
    basic_messages_delay_line = lines[-2]

    hidden_messages_delay = hidden_messages_delay_line.decode()[19:]
    basic_messages_delay = basic_messages_delay_line.decode()[18:]

    return hidden_messages_delay, basic_messages_delay

def main():
    studied_param_value = 0.1
    studied_param_last_value = 10
    studied_param_name = 'hiddenDataAppearance'
    number_of_simulations = 100
    time_for_simulation = 1000

    with open(studied_param_name + 'length_100_apperance_1.csv', 'w') as result_file:
        while studied_param_value < studied_param_last_value:
            params = {
                'hiddenDataAppearance': '-----',
                'channelBandwidth': 1000,
                'hiddenDataSegmentLength': 16,
                'hiddenDataSegmentValue': 100,
                'basicDataAppearance': 1,
                'basicDataLength': 100,
                'timeForGeneratingHiddenMessages': 0,
                'numberOfHiddenMessagesToGenerate': 5,
                'timeForGeneratingBasicMessages': 0,
                'hiddenMessageSegmentLength': 1,
                'numberOfSymulations': number_of_simulations,
                'timeForSimulation': time_for_simulation
            }

            params[studied_param_name] = studied_param_value
            hidden_avg_delay, basic_avg_delay = make_simulation(params)

            result_file.write('{},{},{}\n'.format(studied_param_value, basic_avg_delay, hidden_avg_delay))
            result_file.flush()
            studied_param_value += get_paramater_step(studied_param_value)

main()
