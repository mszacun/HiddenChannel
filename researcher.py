from selenium.webdriver.common.by import By
import json
from backlog.tests.utils.pages.base_page import Page
from selenium import webdriver


class SymulationPage(Page):
    basic_message_avg_delay_selector = '#basic-messages-table .avg-delay'
    hidden_message_avg_delay_selector = '#hidden-messages-table .avg-delay'
    time_selector = '#time'

    basic_message_delay_selector = '#basic-messages-table .delay'
    hidden_message_delay_selector = '#hidden-messages-table .delay'

    base_url = 'file:///home/szacun/sim/simulator.html'

    def __init__(self, webdriver):
        super(SymulationPage, self).__init__(webdriver)

    def go_to_page(self, *args, **kwargs):
        full_url = self.base_url + '#' +  json.dumps(kwargs)
        self.browser.get(full_url)

    def has_all_packets_been_send(self):
        if not self._elements_exists(By.CSS_SELECTOR, self.basic_message_delay_selector) or not self._elements_exists(By.CSS_SELECTOR, self.hidden_message_delay_selector):
            return False

        basic_messages_delay = [td.text for td in
                                self._get_elements(By.CSS_SELECTOR,
                                                   self.basic_message_delay_selector)]
        hidden_messages_delay = [td.text for td in
                                 self._get_elements(By.CSS_SELECTOR,
                                                    self.hidden_message_delay_selector)]

        return all(basic_messages_delay) and all(hidden_messages_delay)

    def get_number_of_hidden_messages(self):
        return len(self._get_elements(By.CSS_SELECTOR, self.hidden_message_delay_selector))

    def get_number_of_basic_messages(self):
        return len(self._get_elements(By.CSS_SELECTOR, self.basic_message_delay_selector))

    def get_avg_basic_messages_delay(self):
        return self._get_text_from_element(By.CSS_SELECTOR, self.basic_message_avg_delay_selector)

    def get_avg_hidden_messages_delay(self):
        return self._get_text_from_element(By.CSS_SELECTOR, self.hidden_message_avg_delay_selector)

    def get_simulation_time(self):
        return self._get_text_from_element(By.CSS_SELECTOR, self.time_selector)


class QuickSymulationPage(SymulationPage):
    basic_message_avg_delay_selector = '#basic-messages-delay'
    hidden_message_avg_delay_selector = '#hidden-messages-delay'
    time_selector = '#time'
    base_url = 'file:///home/szacun/sim/quick_simulator.html'

    def get_number_of_hidden_messages(self):
        return self._get_text_from_element(By.ID, 'hidden-messages-number')

    def get_number_of_basic_messages(self):
        return self._get_text_from_element(By.ID, 'basic-messages-number')

    def has_all_packets_been_send(self):
        return True


def make_simulation(number_of_simulation, progress_file_path, symulation_params):
    browser = webdriver.Chrome('/home/szacun/aginoodle/chromedriver')
    page = QuickSymulationPage(browser)
    number_of_measurements = 0
    hidden_delay_sum = 0.0
    basic_delay_sum = 0.0

    page.go_to_page(**symulation_params)

    with open(progress_file_path, 'w') as progress_file:
        while number_of_measurements < number_of_simulation:
            if page.has_all_packets_been_send():
                number_of_measurements += 1
                hidden_messages_delay = page.get_avg_hidden_messages_delay()
                basic_messages_delay = page.get_avg_basic_messages_delay()
                hidden_delay_sum += float(hidden_messages_delay)
                basic_delay_sum += float(basic_messages_delay)
                hidden_avg_delay = hidden_delay_sum / number_of_measurements
                basic_avg_delay = basic_delay_sum / number_of_measurements

                progress_file.write('{}. Time: {}, hidden: {}({}), basic: {}({}) - current hidden_avg: {}, current base_avg: {}\n'.format(
                    number_of_measurements,
                    page.get_simulation_time(),
                    hidden_messages_delay,
                    page.get_number_of_hidden_messages(),
                    basic_messages_delay,
                    page.get_number_of_basic_messages(),
                    hidden_avg_delay,
                    basic_avg_delay))
                progress_file.flush()

            page.refresh()

    browser.close()
    return (hidden_delay_sum / number_of_measurements, basic_delay_sum / number_of_measurements)

basicDataAppearance = 3


def get_paramater_step(current_value):
    if current_value < basicDataAppearance:
        return 0.1
    if current_value < 7:
        return 0.4

    return 1


def main():

    studied_param_value = 0.2
    studied_param_last_value = 20
    studied_param_name = 'hiddenDataAppearance'

    with open(studied_param_name + '.csv', 'w') as result_file:
        while studied_param_value < studied_param_last_value:
            number_of_simulations = 100 if studied_param_value >= basicDataAppearance else 25
            params = {
                'hiddenDataAppearance': 4,
                'channelBandwith': 1000,
                'hiddenDataSegmentLength': 16,
                'hiddenDataSegmentValue': 5,
                'basicDataAppearance': basicDataAppearance,
                'basicDataLength': 5,
                'timeForGeneratingHiddenMessages': 0,
                'numberOfHiddenMessagesToGenerate': 50,
                'timeForGeneratingBasicMessages': 0,
                'hiddenMessageSegmentLength': 1,
                'animate': False
            }
            params[studied_param_name] = studied_param_value
            progress_file_name = '{}={}'.format(studied_param_name, studied_param_value)

            hidden_avg_delay, basic_avg_delay = make_simulation(number_of_simulations, progress_file_name, params)

            result_file.write('{},{}\n'.format(studied_param_value, basic_avg_delay))
            result_file.flush()
            studied_param_value += get_paramater_step(studied_param_value)


main()
