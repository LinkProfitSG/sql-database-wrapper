#ifndef SQLWRAPPER_HPP
#define SQLWRAPPER_HPP

#include <mariadb/conncpp.hpp>

#include <iostream>
#include <variant>
#include <vector>
#include <type_traits>
#include <functional>
#include <optional>

namespace sqlwrapper {
    enum class PreparedStatementVariant {
        INT, DOUBLE, STRING, BOOLEAN
    };

    class Statement {
    private:
        std::string _statement;
    public:
        Statement(std::string const& statement) : _statement(statement) {}
        Statement() {}

        std::string get_statement() const { return _statement; }

        Statement& update(std::string const& value) {
            _statement.append("UPDATE " + value + " ");
            return *this;
        }
        Statement& delete_sql() {
            _statement.append("DELETE ");
            return *this;
        }
        Statement& from(std::string const& value) {
            _statement.append("FROM " + value + " ");
            return *this;
        }
        Statement& create(std::string const& value) {
            _statement.append("CREATE " + value + " ");
            return *this;
        }
        Statement& insert() { 
            _statement.append("INSERT ");
            return *this;
        }
        Statement& into(std::string const& value) { 
            _statement.append("INTO " + value + " ");
            return *this;
        }
        Statement& values(std::string const& value) {
            _statement.append("VALUES (" + value + ") ");
            return *this;
        }
        Statement& set(std::string const& value) {
            _statement.append("SET " + value + " ");
            return *this;
        }
        Statement& where(std::string const& value) {
            _statement.append("WHERE " + value + " ");
            return *this;
        }
        Statement& select(std::string const& value) {
            _statement.append("SELECT " + value + " ");
            return *this;
        }
    };

    class PreparedStatement {
    private:
        std::shared_ptr<sql::Connection> _connection;
        sql::PreparedStatement* _preparedStatement;
        std::vector<std::variant<int, double, std::string, bool>> _values;
    public:
        PreparedStatement(std::shared_ptr<sql::Connection> connection, Statement const& statement) : _connection(connection) {
            _preparedStatement = _connection->prepareStatement(statement.get_statement());
        }
        ~PreparedStatement() {
            delete _preparedStatement;
        }        

        template<typename... Args>
        PreparedStatement& set_value(Args const&... args) {
            ((_values.emplace_back(args)), ...);
            return *this;
        }

        PreparedStatement& execute(int totalParams) {
            for (int i = 0, currentParam = 1; i < _values.size(); i++, currentParam = (i % totalParams) + 1) {
                switch (static_cast<PreparedStatementVariant>(_values[i].index())) {
                    using enum PreparedStatementVariant;
                    case INT: _preparedStatement->setInt(currentParam, std::get<int>(_values[i]));
                        break;
                    case DOUBLE: _preparedStatement->setDouble(currentParam, std::get<int>(_values[i]));
                        break; 
                    case STRING: _preparedStatement->setString(currentParam, std::get<std::string>(_values[i]));
                        break;
                    case BOOLEAN: _preparedStatement->setBoolean(currentParam, std::get<bool>(_values[i]));
                        break;
                }
            }
            _preparedStatement->execute();
            return *this;
        }
    };

    class ResultSet {
    private:
        sql::ResultSet* _resultSet;
    public:
        ResultSet(sql::ResultSet* resultSet) : _resultSet(resultSet) {}

        ~ResultSet() { delete _resultSet; }

        void get_result(std::function<void(sql::ResultSet*)> fn) {
            while (_resultSet->next()) {
                fn(_resultSet);
            }
        }

        
    };

    class Wrapper {
    private:
        std::shared_ptr<sql::Connection> _connection;
    public:
        Wrapper(std::shared_ptr<sql::Connection> connection) : _connection(connection) {}

        Wrapper& set_schema(std::string const& schema) { 
            _connection->setSchema(schema);
            return *this;
        }

        Wrapper& execute(Statement const& statement) {
            std::unique_ptr<sql::Statement> sqlStatement(_connection->createStatement());
            sqlStatement->execute(statement.get_statement());

            return *this;
        }
        std::unique_ptr<sql::Statement> create_statement(Statement const& statement) {
            std::unique_ptr<sql::Statement> sqlStatement(_connection->createStatement());

            return sqlStatement;
        }

        PreparedStatement prepare_statement(Statement const& statement) {
            return { _connection, statement };
        }
    };
}

#endif