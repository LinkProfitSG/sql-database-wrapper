#include "sqlwrapper.hpp"

int main() {
    try {
        sql::Driver* driver = sql::mariadb::get_driver_instance();
        std::shared_ptr<sql::Connection> connection(driver->connect("jdbc:mariadb://localhost:3306", "root", "admin"));

        sqlwrapper::Wrapper sql(connection);
        sql.set_schema("sql_wrapper_test");

        std::cout << "Schema has been set\n";

        {
            sqlwrapper::Statement statement;
            statement.insert().into("users (name, age)").values("?, ?");

            sqlwrapper::PreparedStatement preparedStatement = sql.prepare_statement(statement);
            preparedStatement.set_value("Someone", 35).execute(2);

            std::cout << "User has been inserted.\n";
        }
    }
    catch (std::exception& err) {
        std::cout << "Error: " << err.what() << '\n';
        return 0;
    }

    //std::cout << "Connected to the database.\n";
    return 0;
}