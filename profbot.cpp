#include <stdio.h>
#include <tgbot/tgbot.h>


const std::string StartMessage = "Я бот для проведения тестирования.\nИспользуйте /test для выбора теста.\nЯ не храню личную информацию и Ваши ответы.";
const std::vector<std::string> bot_commands{"/start","/test","/help"};
int main() {
    std::ifstream tests_file("./testnames");//list of tests
    std::vector<std::string> testnames;
    std::string test_name;
    std::getline(tests_file,test_name);
    while(test_name != "") {
        testnames.push_back(test_name);
        std::getline(tests_file,test_name);
    }
    tests_file.close();
    TgBot::Bot bot("TOKEN");
    TgBot::InlineKeyboardMarkup::Ptr test_keyboard(new TgBot::InlineKeyboardMarkup);
    for (const auto& tname : testnames) {
        std::vector<TgBot::InlineKeyboardButton::Ptr> test_buttons;
        TgBot::InlineKeyboardButton::Ptr test_btn(new TgBot::InlineKeyboardButton);
        test_btn->text = tname;
        test_btn->callbackData = "test_"+tname;
        test_buttons.push_back(test_btn);
        test_keyboard->inlineKeyboard.push_back(test_buttons);
    }

    bot.getEvents().onCommand("start", [&bot](TgBot::Message::Ptr message) {
        bot.getApi().sendMessage(message->chat->id, "Привет! " + StartMessage);
    });
    bot.getEvents().onCommand("test",[&bot,&test_keyboard](TgBot::Message::Ptr message) {
        std::string testname = message -> text;
        int commandlength = 6;
        if(testname.length() < commandlength) {
            bot.getApi().sendMessage(message->chat->id, "Выберите один из представленных ниже тестов:", false, 0, test_keyboard);
            return;
        }
        testname = &(message->text[commandlength]);

        bot.getApi().sendMessage(message->chat->id,"\""+testname+"\"");
    });
    bot.getEvents().onCommand("help",[&bot](TgBot::Message::Ptr message) {
        std::string testname = message -> text;
        bot.getApi().sendMessage(message->chat->id,StartMessage);
    });
    bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
        printf("User wrote %s\n", message->text.c_str());
        for (const auto& bot_command : bot_commands) {
            if (StringTools::startsWith(message->text, bot_command)) {
                return;
            }
        }
        bot.getApi().sendMessage(message->chat->id, "Я не знаю команду " + message->text + ". \nИспользуйте /help для вывода информации о возможных командах");
    });
    bot.getEvents().onCallbackQuery([&bot,&test_keyboard] (TgBot::CallbackQuery::Ptr query) {
        std::vector<std::string> call_data = StringTools::split(query->data,'_');
        //test chosen
        printf("Callback: %s\n", query->data.c_str());
        if (call_data[0] == "test") {
            std::ifstream test_file("./tests/" + call_data[1] + "/desc");
            if (!test_file.is_open()) {
                bot.getApi().sendMessage(query->message->chat->id, "Не могу открыть тест. Попробуйте другой вариант.");
                return;
            }
            std::string test_description;
            std::getline(test_file,test_description);
            int quest_count, result_count;
            test_file >> quest_count >> result_count;
            TgBot::InlineKeyboardMarkup::Ptr start_test_keyboard(new TgBot::InlineKeyboardMarkup);
            std::vector<TgBot::InlineKeyboardButton::Ptr> start_test_btns;
            TgBot::InlineKeyboardButton::Ptr start_test_btn(new TgBot::InlineKeyboardButton());
            start_test_btn->text = "Начать тестирование";
            start_test_btn->callbackData = "ans_" + call_data[1] + "_1_" + std::to_string(quest_count) + '_' + std::to_string(result_count);
            for (int i = 0; i < result_count; i++){
                start_test_btn->callbackData.append("_0");
            }
            start_test_btns.push_back(start_test_btn);
            start_test_keyboard->inlineKeyboard.push_back(start_test_btns);
            bot.getApi().sendMessage(query->message->chat->id, test_description.c_str(), false, 0, start_test_keyboard);
            test_file.close();
            return;
        }
        //answer chosen
        if (call_data[0] == "ans") {
            int quest_num = stoi(call_data[2]);
            int quest_count = stoi(call_data[3]);
            int result_count = stoi(call_data[4]);
            std::vector<int> curr_results;
            for (int i = 0; i < result_count; i++){
                curr_results.push_back(stoi(call_data[5+i]));
            }
            std::ifstream quest_file("./tests/" + call_data[1] + "/quest/" + call_data[2]);
            if (!quest_file.is_open()){
                int max_result = 0;
                for (int i = 1; i < result_count; i++){
                    if (curr_results[i] > curr_results[max_result]){
                        max_result = i;
                    }
                }
                std::ifstream result_file("./tests/" + call_data[1] + "/results/" + std::to_string(max_result+1));
                std::string result_text;
                getline(result_file,result_text);
                bot.getApi().editMessageText("8D", query->message->chat->id, query->message->messageId);
                bot.getApi().editMessageText(("Тест \"" + call_data[1] +"\" завершён. Результат:\n"+result_text).c_str(),query->message->chat->id, query->message->messageId);
                result_file.close();
                return;
            }
            std::string questtext, ans_cstr;
            std::getline(quest_file, questtext);
            int ans_count;
            std::getline(quest_file, ans_cstr);
            ans_count = stoi(ans_cstr);
            quest_num++;
            TgBot::InlineKeyboardMarkup::Ptr quest_keyboard(new TgBot::InlineKeyboardMarkup);
            std::vector <TgBot::InlineKeyboardButton::Ptr> answers;
            for (int i = 0; i < ans_count; i++) {
                std::string ans;
                std::getline(quest_file, ans);
                TgBot::InlineKeyboardButton::Ptr ans_btn(new TgBot::InlineKeyboardButton());
                size_t ans_start;
                int ans_result = std::stoi(ans,&ans_start)-1;
                ans_btn->text = std::to_string(i+1);
                questtext.append("\n" + std::to_string(i+1) + ") " + &ans[ans_start]);
                ans_btn->callbackData = "ans_" + call_data[1] + '_'+ std::to_string(quest_num)+'_' + call_data[3] + '_' + call_data[4];
                int curr_result;
                for (int j = 0; j < result_count; j++){
                    curr_result = curr_results[j];
                    if (ans_result == j) curr_result++;
                    ans_btn->callbackData.append('_'+std::to_string(curr_result));
                }
                answers.push_back(ans_btn);
            }
            quest_keyboard->inlineKeyboard.push_back(answers);
            if (questtext != query->message->text) {
                bot.getApi().editMessageText("8)", query->message->chat->id, query->message->messageId);
                bot.getApi().editMessageText(questtext.c_str(), query->message->chat->id, query->message->messageId, "","", false, quest_keyboard);
            }
            quest_file.close();
        }
    });
    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
        }
    } catch (TgBot::TgException& e) {
        printf("error: %s\n", e.what());
    }
    return 0;
}
