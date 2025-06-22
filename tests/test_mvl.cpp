// _______________ TEST FRAMEWORK & MODULE  _______________

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "thirdparty/doctest.h"

#include "test.hpp"

#include "UTL/mvl.hpp"

// _______________________ INCLUDES _______________________

#include <algorithm>
#include <array>
#include <cstddef>
#include <execution>
#include <functional>
#include <numeric>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <vector>

// ____________________ DEVELOPER DOCS ____________________

// NOTE: DOCS

// ____________________ IMPLEMENTATION ____________________

// ====================
// --- Test helpers ---
// ====================

// Helper macro for checking whole matrices against a "target" matrix
template <typename T, mvl::Dimension dimension, mvl::Type type, mvl::Ownership ownership, mvl::Checking checking,
          mvl::Layout layout>
bool check_matrix_impl(const mvl::GenericTensor<T, dimension, type, ownership, checking, layout>& checked,
                       const mvl::Matrix<T>&                                                      target) {
    // Check rows dim
    const bool correct_rows = target.rows() == checked.rows();
    if (!correct_rows)
        std::cout << "Incorrect rows: target = " << target.rows() << ", checked = " << checked.rows() << "\n";

    // Check cols dim
    const bool correct_cols = target.cols() == checked.cols();
    if (!correct_cols)
        std::cout << "Incorrect cols: target = " << target.cols() << ", checked = " << checked.cols() << "\n";

    // Check tensor contents
    bool correct_contents = true;
    // .for_each() takes care of sparse matrices by only comparing their contained elements
    checked.for_each([&](const T& elem, size_t i, size_t j) {
        if (elem != target(i, j)) {
            correct_contents = false;
            std::cout << "Incorrect contents: target(" << i << ", " << j << ") = " << target(i, j) << ", checked(" << i
                      << ", " << j << ") = " << elem << "\n";
        }
    });

    return correct_rows && correct_cols && correct_contents;
}

#define CHECK_MATRIX(...) CHECK(check_matrix_impl(__VA_ARGS__))

// ====================
// --- Matrix tests ---
// ====================

// TODO: Restructure and cleanup one the API gets finalized

TEST_CASE("Sparse matrix basic functionality works as expected") {
    // Build sparse matrix + insert some more
    mvl::SparseMatrix<int> mat(4, 4,
                               {
                                   {0, 0, 10},
                                   {1, 1, 20},
                                   {2, 2, 30},
                                   {3, 3, 40}
    });

    mat.insert_triplets({
        {0, 3, 50}
    });

    // Check basic assumptions
    CHECK(mat.size() == 5);
    CHECK(mat(0, 0) == 10);
    CHECK(mat(1, 1) == 20);
    CHECK(mat(2, 2) == 30);
    CHECK(mat(3, 3) == 40);
    CHECK(mat(0, 3) == 50);
    CHECK(mat.contains_index(0, 3) == true);
    CHECK(mat.contains_index(0, 2) == false);
    CHECK(mat.sum() == 10 + 20 + 30 + 40 + 50);

    // Check after erasing a few triplets
    mat.erase_triplets({
        {0, 0},
        {1, 1}
    });

    CHECK(mat.size() == 3);
    CHECK(mat(2, 2) == 30);
    CHECK(mat(3, 3) == 40);
    CHECK(mat(0, 3) == 50);
    CHECK(mat.sum() == 30 + 40 + 50);
}

TEST_CASE("Strided view passes basic sanity checks") {
    std::vector<int> vec = {1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3,
                            1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3};

    constexpr size_t rows  = 3;
    constexpr size_t cols  = 4;
    constexpr size_t size  = 12;
    constexpr size_t chunk = 3; // width of a repeating chink

    // Unit stride
    mvl::StridedMatrixView<int> trivial_view(rows, cols * chunk, 0, 1, vec.data());

    CHECK(trivial_view[0] == 1);
    CHECK(trivial_view[1] == 2);
    CHECK(trivial_view[2] == 3);

    CHECK(trivial_view(0, 0) == 1);
    CHECK(trivial_view(0, 1) == 2);
    CHECK(trivial_view(0, 2) == 3);
    CHECK(trivial_view(2, 0) == 1);
    CHECK(trivial_view(2, 1) == 2);
    CHECK(trivial_view(2, 2) == 3);

    // Non-unit col stride
    mvl::StridedMatrixView<int> view_plus0(rows, cols, 0, chunk, vec.data() + 0);
    mvl::StridedMatrixView<int> view_plus1(rows, cols, 0, chunk, vec.data() + 1);
    mvl::StridedMatrixView<int> view_plus2(rows, cols, 0, chunk, vec.data() + 2);

    mvl::Matrix<int> expected_2 = {
        {3, 3, 3, 3},
        {3, 3, 3, 3},
        {3, 3, 3, 3}
    };

    CHECK(view_plus0.sum() == size * 1);
    CHECK(view_plus1.sum() == size * 2);
    CHECK(view_plus2.sum() == size * 3);

    CHECK(view_plus2.true_for_any([&](const int& e, size_t, size_t) { return e == 3; }));
    CHECK(view_plus2.true_for_all([&](const int& e, size_t, size_t) { return e != 2; }));

    CHECK(view_plus2.compare_contents(expected_2));

    for (const auto& val : view_plus2.to_std_vector()) { CHECK(val == 3); }

    CHECK(view_plus2.contains(2) == false);
    CHECK(view_plus2.contains(3) == true);

    CHECK(view_plus2.count(2) == 0);
    CHECK(view_plus2.count(3) == size);

    // Non-zero row stride
    mvl::StridedMatrixView<int> view_with_row_stride(2, cols, cols * chunk, chunk, vec.data() + 2);

    for (const auto& val : view_with_row_stride) { CHECK(val == 3); }

    struct Val {
        int x;
    };
    mvl::Matrix<Val> mat;
    mat.stable_sort([](const Val& l, const Val& r) -> bool { return l.x < r.x; });
}

TEST_CASE("Matrix constructors & methods derived from storage::AbstractIndexableObject behave as expected") {

    SUBCASE("Matrix default-initialization is correct") {
        mvl::Matrix<int> matrix(12, 5);
        bool             all_elements_are_zero = true;
        matrix.for_each([&](int& element) {
            if (element != 0) all_elements_are_zero = false;
        });
        CHECK(all_elements_are_zero);
    }


    SUBCASE("Matrix value-initialization is correct") {
        mvl::Matrix<std::string> matrix(12, 5, "xo");
        bool                     all_elements_are_correct = true;
        matrix.for_each([&](std::string& element) {
            if (element != "xo") all_elements_are_correct = false;
        });
        CHECK(all_elements_are_correct);
    }


    SUBCASE("Matrix 1D indexation") {
        constexpr std::size_t rows = 3, cols = 2, size = rows * cols;
        mvl::Matrix<int>      matrix(rows, cols, 1);
        const auto&           cref = matrix;
        // Reading
        int                   sum  = 0;
        for (std::size_t idx = 0; idx < size; ++idx) sum += cref[idx];
        CHECK(sum == size);
        // Writing
        sum = 0;
        for (std::size_t idx = 0; idx < size; ++idx) sum += (matrix[idx] = -1); // also tests operator=() returns
        CHECK(-sum == size);
    }


    SUBCASE("Matrix 2D indexation") {
        constexpr std::size_t        rows = 3, cols = 4, size = rows * cols;
        mvl::Matrix<float>           matrix(rows, cols, 0.5f);
        const auto&                  cref = matrix;
        // Reading
        decltype(matrix)::value_type sum  = 0.0f; // also tests the types
        for (std::size_t i = 0; i < rows; ++i)
            for (std::size_t j = 0; j < cols; ++j) sum += cref(i, j);
        CHECK(sum == doctest::Approx(size * 0.5f));
        // Writing
        sum = 0.0f;
        for (std::size_t i = 0; i < rows; ++i)
            for (std::size_t j = 0; j < cols; ++j) sum += (matrix(i, j) = 1.4f);
        CHECK(sum == doctest::Approx(size * 1.4f));
    }


    SUBCASE("Matrix .fill() + .for_each() + .size() test") {
        // .fill() matrix with 1's and check that their .for_each()-computed sum equals .size()
        auto matrix = std::move(mvl::Matrix<int>(15, 7).fill(1));
        // triggers copy without std::move()
        // By doing this with 'auto' we test .fill() CRTP return type and Matrix move operator=
        int  sum    = 0;
        matrix.for_each([&](int& element) { sum += element; });
        CHECK(sum == matrix.size());
    }


    SUBCASE("Matrix .for_each() (with idx) test") {
        // Fill matrix elements with their corresponding 1D indexes and check that it is correct
        auto matrix = mvl::Matrix<int>(3, 5);
        matrix.for_each([](int& element, std::size_t idx) { element = idx; });
        for (std::size_t i = 0; i < matrix.size(); ++i) CHECK(matrix[i] == i);
    }

    SUBCASE("Matrix .for_each() (with i, j) test") {
        // Fill matrix elements with their corresponding 1D indexes and check that it is correct
        auto matrix = mvl::Matrix<int>(3, 5);
        matrix.for_each([](int& element, std::size_t i, std::size_t j) { element = 10 * i + j; });
        for (std::size_t i = 0; i < matrix.rows(); ++i)
            for (std::size_t j = 0; j < matrix.cols(); ++j) CHECK(matrix(i, j) == (10 * i + j));
    }

    SUBCASE("Matrix .front() and .back() test") {
        // Fill matrix elements with their corresponding 1D indexes and check that it is correct
        mvl::Matrix<char> matrix(4, 13);
        matrix.front() = 'F';
        matrix.back()  = 'B';
        CHECK(matrix[0] == matrix.front());
        CHECK(matrix[0] == 'F');
        CHECK(matrix[matrix.size() - 1] == matrix.back());
        CHECK(matrix[matrix.size() - 1] == 'B');
    }

    SUBCASE("Matrix .to_std_vector() + initializer_list constructor test") {
        mvl::Matrix<char> matrix = {
            {'a', 'b'},
            {'c', 'd'}
        };
        CHECK(matrix.to_std_vector() == std::vector{'a', 'b', 'c', 'd'});
    }


    SUBCASE("Matrix initializer_list constructor (throw case) test") {
        bool caught_appropriate_exception = false;
        try {
            mvl::Matrix<int> matrix({
                {1, 2, 3, 4},
                {5, 6, 7}, // missing an element
                {8, 9, 10, 11}
            });
        } catch (std::invalid_argument const& ex) {
            caught_appropriate_exception = true;
            INFO("Caught exception: ", ex.what(), "\n");
        }
        CHECK(caught_appropriate_exception);
    }

    SUBCASE("Matrix const & non-const overloads test") {
        mvl::Matrix<int> matrix = {
            {1, 2},
            {3, 4},
            {5, 6}
        };
        const auto& cref = matrix;
        CHECK(matrix[0] == cref[0]);
        CHECK(matrix(0, 1) == cref(0, 1));
        CHECK(matrix.front() == cref.front());
        CHECK(matrix.back() == cref.back());
    }
}

TEST_CASE("Basic matrix methods behave as expected") {

    SUBCASE("Matrix 2D bound-checking test") {
        mvl::Matrix<int, mvl::Checking::BOUNDS> matrix(4, 5);
        // 1D access
        auto                                    access_idx_throws = [&](std::size_t idx) -> bool {
            try {
                matrix[idx] = 0;
            } catch (std::out_of_range const& ex) {
                INFO("Caught exception: ", ex.what(), "\n");
                return true;
            }
            return false;
        };
        CHECK(access_idx_throws(0) == false);
        CHECK(access_idx_throws(7) == false);
        CHECK(access_idx_throws(19) == false);
        CHECK(access_idx_throws(20) == true);
        CHECK(access_idx_throws(34) == true);
        // 2D access
        auto access_ij_throws = [&](std::size_t i, std::size_t j) -> bool {
            try {
                matrix(i, j) = 0;
            } catch (std::out_of_range const& ex) {
                INFO("Caught exception: ", ex.what(), "\n");
                return true;
            }
            return false;
        };
        CHECK(access_ij_throws(0, 0) == false);
        CHECK(access_ij_throws(3, 4) == false);
        CHECK(access_ij_throws(2, 1) == false);
        CHECK(access_ij_throws(4, 0) == true);
        CHECK(access_ij_throws(0, 5) == true);
        CHECK(access_ij_throws(10, 15) == true);
    }

    SUBCASE("Matrix CHECKED -> UNCHECKED conversion test") {
        mvl::Matrix<int, mvl::Checking::BOUNDS> checked_matrix = {
            {1, 2},
            {3, 4}
        };
        mvl::Matrix<int, mvl::Checking::NONE> unchecked_matrix;

        // CHECKED -> UNCHECKED move
        CHECK(checked_matrix.empty() == false);
        CHECK(unchecked_matrix.empty() == true);

        // unchecked_matrix = checked_matrix; <- will not work, conversion over boundaries has to be explicit!
        unchecked_matrix = mvl::Matrix<int, mvl::Checking::NONE>(std::move(checked_matrix));

        // NOTE: No guarantees about moved-from matrix being empty!
        CHECK(unchecked_matrix.empty() == false);

        CHECK(unchecked_matrix(0, 0) == 1);
        CHECK(unchecked_matrix(0, 1) == 2);
        CHECK(unchecked_matrix(1, 0) == 3);
        CHECK(unchecked_matrix(1, 1) == 4);
    }
}


TEST_CASE("Matrix views behave as expected") {


    SUBCASE("Can create const matrix view from matrix and read the contents") {
        mvl::Matrix<int> matrix = {
            {1, 2},
            {3, 4}
        };
        mvl::ConstMatrixView<int> view(matrix);

        // Indexing a view gives the same values
        CHECK(view(0, 0) == 1);
        CHECK(view(0, 1) == 2);
        CHECK(view(1, 0) == 3);
        CHECK(view(1, 1) == 4);
        // Sensible assumptions work
        CHECK(view.data() == matrix.data());
        CHECK(view.to_std_vector() == matrix.to_std_vector());
    }

    SUBCASE("Can create const matrix view from matrix and read the contents") {
        constexpr std::size_t rows = 3, cols = 4, size = rows * cols;
        std::vector<int>      original_vector(size, 17);
        const int*            const_ptr_to_data = original_vector.data();

        // Create view to raw data and use it like a matrix
        mvl::ConstMatrixView<int> view(rows, cols, const_ptr_to_data);
        // All elements are correct
        for (std::size_t i = rows; i < rows; ++i)
            for (std::size_t j = cols; j < cols; ++j) CHECK(view(i, j) == 17);
        // Sensible assumptions work
        CHECK(view.data() == original_vector.data());
        CHECK(view.to_std_vector() == original_vector);
        // Const version of .for_each
        decltype(view)::value_type sum = 0;
        view.for_each([&](const int& element) { sum += element; });
        CHECK(sum == size * 17);
    }


    SUBCASE("Can create mutable matrix view and change the contents of matrix") {
        mvl::Matrix<int> matrix = {
            {1, 2},
            {3, 4}
        };

        mvl::MatrixView<int> view = matrix;

        // Indexing a view gives the same values
        CHECK(view(0, 0) == 1);
        CHECK(view(0, 1) == 2);
        CHECK(view(1, 0) == 3);
        CHECK(view(1, 1) == 4);
        // Sensible assumptions work
        CHECK(view.data() == matrix.data());
        CHECK(view.to_std_vector() == matrix.to_std_vector());
        // Can modify matrix through a view
        view.fill(7);
        matrix.for_each([](int& element) { CHECK(element == 7); });
        view[0]     = 5;
        view(1, 0)  = 3;
        view.back() = 7;
        CHECK(matrix(0, 0) == 5);
        CHECK(matrix(1, 0) == 3);
        CHECK(matrix(1, 1) == 7);
    }


    SUBCASE("Can create bound-checked view from unchecked matrix") {
        mvl::Matrix<int, mvl::Checking::NONE> unchecked_matrix = {
            {1, 2},
            {3, 4}
        };

        // Create bound-checked view to matrix
        mvl::MatrixView<int, mvl::Checking::BOUNDS> checked_view = unchecked_matrix;

        auto access_ij_throws = [&](std::size_t i, std::size_t j) -> bool {
            try {
                checked_view(i, j) = 0;
            } catch (std::out_of_range const& ex) {
                INFO("Caught exception: ", ex.what(), "\n");
                return true;
            }
            return false;
        };
        CHECK(access_ij_throws(0, 0) == false);
        CHECK(access_ij_throws(1, 0) == false);
        CHECK(access_ij_throws(1, 1) == false);
        CHECK(access_ij_throws(2, 0) == true);
        CHECK(access_ij_throws(0, 2) == true);
        CHECK(access_ij_throws(13, 14) == true);
    }


    SUBCASE("Can create view from const matrix reference and chain functions") {
        mvl::Matrix<int> matrix = {
            {1, 2},
            {3, 4}
        };
        const auto& cref = matrix;

        // Create view from const reference
        mvl::ConstMatrixView<int> view = cref;
        // Basic assumptions
        CHECK(view.rows() == matrix.rows());
        CHECK(view.cols() == matrix.cols());
        CHECK(view.size() == matrix.size());
        CHECK(view.data() == matrix.data());
        // Try abusing function chains
        int sum = 1;
        CHECK(matrix.for_each([&](const int& element, std::size_t idx) { sum *= element * (idx + 1); })
                  .for_each([&](const int& element, std::size_t) { sum -= element; })
                  .back() == 4);
        CHECK(sum == 566); // computed by hand
    }
}


TEST_CASE("Iterators behave as expected") {


    SUBCASE("Const iterator works") {
        mvl::Matrix<int> matrix(4, 5);
        matrix.for_each([](int& element, std::size_t idx) { element = idx; });
        const auto& cref = matrix;
        // Check that all filled values are read the same when const-iterating though
        std::size_t idx  = 0;
        for (auto it = cref.cbegin(); it != cref.cend(); ++it, ++idx) CHECK(*it == idx);
        // Check that we covered the whole array
        CHECK(idx == cref.size());
        // Check basic operations of random access iterator
        CHECK(*cref.cbegin() == cref.front());             // *it
        CHECK(cref.cend() - cref.cbegin() == cref.size()); // it - it
        CHECK(*(cref.cbegin() + 1) == 1);                  // it + n
        CHECK(*(2 + cref.cbegin()) == 2);                  // n + it
        CHECK(*(cref.cend() - 1) == cref.back());          // it - n
        CHECK(cref.cbegin()[2] == 2);                      // it[n]
        CHECK(cref.cbegin() < cref.cend());                // it1 < it2
        CHECK(cref.cend() > cref.cbegin());                // it1 > it2
        CHECK(cref.cbegin() <= cref.cend());               // it1 <= it2
        CHECK(cref.cend() >= cref.cbegin());               // it1 >= it2
        CHECK(*(cref.cbegin() += 2) == 2);                 // it1 += n
        CHECK(*(cref.cend() -= 1) == matrix.back());       // it1 -= n
        // Check that operations don't mess up when we use derived reverse iterator
        CHECK(cref.crbegin() < cref.crend()); // it1 < it2
        CHECK(cref.crend() > cref.crbegin()); // it1 > it2
        // Iterate in reverse
        matrix.for_each([&](int& element, std::size_t idx) { element = idx; }); // [ 0 .. N-1 ]
        for (auto it = matrix.rbegin(); it != matrix.rend(); ++it)
            CHECK(matrix.rend() - 1 - it == *it); // true for how we filled matrix
        // note that reverse_iterator also reverses the logic of 'it1 - it2', makes sense because '.crbegin() <
        // .crend()'
    }


    SUBCASE("Mutable iterator works") {
        mvl::Matrix<int> matrix(4, 5);
        // Check that range-based for now works (it's a syntactic sugar defined for all containers with
        // defined forward iterators and .begin(), .end() methods) (doesn't call .cbegin() & .cend() even if they exist)
        for (auto& element : matrix) element = 7;
        for (const auto& element : matrix) CHECK(element == 7);
        // Check basic operations of random access iterator
        matrix.for_each([](int& element, std::size_t idx) { element = idx; });
        CHECK(*matrix.begin() == matrix.front());              // *it
        CHECK(matrix.end() - matrix.begin() == matrix.size()); // it - it
        CHECK(*(matrix.begin() + 1) == 1);                     // it + n
        CHECK(*(2 + matrix.begin()) == 2);                     // n + it
        CHECK(*(matrix.end() - 1) == matrix.back());           // it - n
        CHECK(matrix.begin()[2] == 2);                         // it[n]
        CHECK(matrix.begin() < matrix.end());                  // it1 < it2
        CHECK(matrix.end() > matrix.begin());                  // it1 > it2
        CHECK(matrix.begin() <= matrix.end());                 // it1 <= it2
        CHECK(matrix.end() >= matrix.begin());                 // it1 >= it2
        CHECK(*(matrix.begin() += 2) == 2);                    // it1 += n
        CHECK(*(matrix.end() -= 1) == matrix.back());          // it1 -= n
        // Check that operations don't mess up when we use derived reverse iterator
        CHECK(matrix.rbegin() < matrix.rend()); // it1 < it2
        CHECK(matrix.rend() > matrix.rbegin()); // it1 > it2
        // Check that algorithms works
        // std::sort
        matrix.for_each([&](int& element, std::size_t idx) { element = matrix.size() - 1 - idx; }); // [ N-1 .. 0 ]
        std::sort(matrix.begin(), matrix.end()); // becomes [ 0 ... N-1 ]
        matrix.for_each([&](int& element, std::size_t idx) { CHECK(element == idx); });
        // std::fill + std::accumulate
        std::fill(matrix.begin(), matrix.end(), 10);
        const int sum_1 = std::accumulate(matrix.begin(), matrix.end(), 0);
        int       sum_2 = 0;
        matrix.for_each([&](const int& element) { sum_2 += element; });
        CHECK(sum_1 == sum_2);
        // Iterate in reverse
        matrix.for_each([&](int& element, std::size_t idx) { element = matrix.size() - 1 - idx; }); // [ N-1 .. 0 ]
        for (auto it = matrix.rbegin(); it != matrix.rend(); ++it)
            CHECK(it - matrix.rbegin() == *it); // true for how we filled matrix
    }
}


TEST_CASE("Col-major ordering and transposition behave as expected") {


    SUBCASE("matrix.transposed() behaves as expected") {
        mvl::Matrix<int> matrix(3, 5, 0);
        matrix.for_each([](int& element, std::size_t i, std::size_t j) { element = 1000 * i + j; });
        // Transpose
        auto matrixT = matrix.transposed();
        // Check sizes
        CHECK(matrix.rows() == matrixT.cols());
        CHECK(matrix.cols() == matrixT.rows());
        // Check contents
        for (std::size_t i = 0; i < matrix.rows(); ++i)
            for (std::size_t j = 0; j < matrix.cols(); ++j) CHECK(matrix(i, j) == matrixT(j, i));
    }


    SUBCASE("Col-major matrix satisfied basic assumptions") {
        mvl::Matrix<int, mvl::Checking::NONE, mvl::Layout::RC> row_matrix = {
            {0, 1,  2,  3},
            {4, 5,  6,  7},
            {8, 9, 10, 11}
        };
        mvl::Matrix<int, mvl::Checking::NONE, mvl::Layout::CR> col_matrix = {
            {0, 1,  2,  3},
            {4, 5,  6,  7},
            {8, 9, 10, 11}
        };

        // Basic assumptions
        CHECK(row_matrix.rows() == col_matrix.rows());
        CHECK(row_matrix.cols() == col_matrix.cols());
        CHECK(row_matrix.size() == col_matrix.size());
        // Extents
        CHECK(row_matrix.extent_major() == row_matrix.rows());
        CHECK(row_matrix.extent_minor() == row_matrix.cols());
        CHECK(col_matrix.extent_major() == col_matrix.cols());
        CHECK(col_matrix.extent_minor() == col_matrix.rows());
        // Filled with the values we expect
        col_matrix.for_each([&](int& element, std::size_t i, std::size_t j) { CHECK(element == row_matrix(i, j)); });
        for (std::size_t i = 0; i < col_matrix.rows(); ++i)
            for (std::size_t j = 0; j < col_matrix.rows(); ++j) CHECK(row_matrix(i, j) == col_matrix(i, j));
        // Check memory layout
        auto row_matrix_transposed = row_matrix.transposed();
        for (std::size_t idx = 0; idx < col_matrix.size(); ++idx) CHECK(col_matrix[idx] == row_matrix_transposed[idx]);
    }


    SUBCASE("Col-major to row-major conversion test") {
        mvl::Matrix<int, mvl::Checking::NONE, mvl::Layout::CR> initial_col_matrix = {
            {0, 1,  2,  3},
            {4, 5,  6,  7},
            {8, 9, 10, 11}
        };

        // TEMP:
        // Commented out while writing ctors

        // col-major -> row-major
        mvl::Matrix<int, mvl::Checking::NONE, mvl::Layout::RC> row_matrix(initial_col_matrix);
        // row-major -> col-major
        mvl::Matrix<int, mvl::Checking::NONE, mvl::Layout::CR> col_matrix(row_matrix);

        for (std::size_t i = 0; i < col_matrix.rows(); ++i)
            for (std::size_t j = 0; j < col_matrix.rows(); ++j) {
                CHECK(row_matrix(i, j) == initial_col_matrix(i, j));
                CHECK(row_matrix(i, j) == col_matrix(i, j));
            }
    }


    SUBCASE("Col-major view") {
        mvl::Matrix<int> matrix = {
            {0, 1,  2,  3},
            {4, 5,  6,  7},
            {8, 9, 10, 11}
        };

        mvl::ConstMatrixView<int, mvl::Checking::NONE, mvl::Layout::RC> row_view(matrix.rows(), matrix.cols(),
                                                                                 matrix.data());
        mvl::ConstMatrixView<int, mvl::Checking::NONE, mvl::Layout::CR> col_view(matrix.rows(), matrix.cols(),
                                                                                 matrix.data());

        // std::cout << "row_view:\n" << row_view.dump() << "col_view:\n" << col_view.dump();

        // Check contents
        CHECK(row_view(0, 0) == 0);
        CHECK(row_view(0, 1) == 1);
        CHECK(row_view(0, 2) == 2);
        CHECK(row_view(0, 3) == 3);

        CHECK(col_view(0, 0) == 0);
        CHECK(col_view(1, 0) == 1);
        CHECK(col_view(2, 0) == 2);
    }
}

TEST_CASE("Matrix sparse subview test") {
    mvl::Matrix<int> mat = {
        {8, 7, 7, 7, 8, 0}, //
        {7, 8, 0, 0, 8, 0}, //
        {7, 0, 8, 0, 8, 0}, //
        {7, 0, 0, 8, 8, 0}, //
        {3, 3, 3, 3, 8, 0}  //
    };

    // Test mutable filtering
    const auto view_1 = mat.filter([](const int& elem) { return elem == 8; }).fill(10);
    const auto view_2 = mat.filter([](const int& elem) { return elem == 3; }).fill(20);
    const auto view_3 = mat.filter([](const int& elem) { return elem == 7; }).fill(30);

    CHECK(view_1.size() == 9);
    CHECK(view_2.size() == 4);
    CHECK(view_3.size() == 6);
    CHECK(view_1.size() * 10 == view_1.sum());
    CHECK(view_2.size() * 20 == view_2.sum());
    CHECK(view_3.size() * 30 == view_3.sum());

    // Test const filtering
    const auto& cref = mat;

    const auto const_view_1        = cref.filter([](const int& elem) { return elem == 10; });
    const auto const_view_2        = cref.filter([](const int& elem) { return elem == 20; });
    const auto const_view_3        = cref.filter([](const int& elem) { return elem == 30; });
    const auto const_diagonal_view = cref.diagonal();

    CHECK(const_view_1.size() == 9);
    CHECK(const_view_2.size() == 4);
    CHECK(const_view_3.size() == 6);
    CHECK(const_diagonal_view.size() == 5);
    CHECK(const_view_1.size() * 10 == const_view_1.sum());
    CHECK(const_view_2.size() * 20 == const_view_2.sum());
    CHECK(const_view_3.size() * 30 == const_view_3.sum());
    CHECK(const_diagonal_view.size() * 10 == const_diagonal_view.sum());

    auto middle_block = cref.block(1, 1, 3, 4);
    CHECK(middle_block.rows() == 3);
    CHECK(middle_block.cols() == 4);
    CHECK(middle_block.size() == 12);
    CHECK(middle_block(0, 0) == 10);
    CHECK(middle_block(1, 1) == 10);
    CHECK(middle_block(2, 2) == 10);
    CHECK(middle_block(0, 1) == 0);
    CHECK(middle_block(0, 2) == 0);
    CHECK(middle_block(0, 3) == 10);
    // const auto eights_diagonal = eights.filter([](const int &, size_t i, size_t j){ return i == j; });
}

TEST_CASE("Matrix block subview case") {
    mvl::Matrix<int> matrix = {
        { 1,  2,  3,  4,  5, 0}, //
        { 7,  8,  0,  0, 11, 0}, //
        {13,  0, 15,  0, 17, 0}, //
        {19,  0,  0, 22, 23, 0}, //
        {25, 26, 27, 28, 29, 0}  //
    };

    // Build a strided view
    mvl::ConstStridedMatrixView<int> strided_view(3, 3, 1 * matrix.cols(), 2, matrix.data()); // non-trivial strides

    CHECK_MATRIX(strided_view, {
                                   { 1,  3,  5}, //
                                   {13, 15, 17}, //
                                   {25, 27, 29}  //
    });

    // Check strided view blocking subview
    auto strided_view_block = strided_view.block(1, 1, 2, 2);

    CHECK_MATRIX(strided_view_block, {
                                         {15, 17}, //
                                         {27, 29}  //
    });

    // Build a sparse view
    auto sparse_view = matrix.filter([](int elem) { return elem != 0; });

    CHECK_MATRIX(sparse_view, {
                                  { 1,  2,  3,  4,  5, 0}, //
                                  { 7,  8,  0,  0, 11, 0}, //
                                  {13,  0, 15,  0, 17, 0}, //
                                  {19,  0,  0, 22, 23, 0}, //
                                  {25, 26, 27, 28, 29, 0}  //
    });

    // Check sparse blocking subview
    auto sparse_view_block = sparse_view.block(0, 1, 3, 3);

    CHECK_MATRIX(sparse_view_block, {
                                        {2,  3, 4}, //
                                        {8,  0, 0}, //
                                        {0, 15, 0}, //
    });

    // Check other block views on the original matrix
    CHECK_MATRIX(matrix.row(3), {
                                    {19, 0, 0, 22, 23, 0}
    });

    CHECK_MATRIX(matrix.col(2), {
                                    {3},  //
                                    {0},  //
                                    {15}, //
                                    {0},  //
                                    {27}  //
                                });
}

TEST_CASE("Testing algebraic operations") {
    mvl::SparseMatrix<int> A(3, 3,
                             {
                                 {0, 0, 10},
                                 {1, 1, 20},
                                 {2, 2, 30}
    });

    const mvl::SparseMatrix<int> B(3, 3,
                                   {
                                       {0, 2, 3},
                                       {1, 2, 6},
                                       {2, 2, 9}
    });

    mvl::Matrix<int> C = {
        {0, 2, 4}, //
        {0, 6, 8}, //
        {0, 0, 0}  //
    };

    const mvl::Matrix<int> D = {
        {1, 0, 0}, //
        {2, 0, 0}, //
        {3, 2, 1}  //
    };

    // Sparse / sparse operators
    CHECK_MATRIX(A + B, {
                            {10,  0,  3}, //
                            { 0, 20,  6}, //
                            { 0,  0, 39}, //
    });

    CHECK_MATRIX(A.clone() += B, {
                                     {10,  0,  3}, //
                                     { 0, 20,  6}, //
                                     { 0,  0, 39}, //
    });

    // Dense / dense & dense / sparse operators
    CHECK_MATRIX(D + C + D + D - C + C + A, {
                                                {13,  2,  4}, //
                                                { 6, 26,  8}, //
                                                { 9,  6, 33}, //
    });

    // Dense / sparse & sparse / dense operators
    CHECK_MATRIX(A + C + A, {
                                {20,  2,  4}, //
                                { 0, 46,  8}, //
                                { 0,  0, 60}, //
    });

    CHECK_MATRIX(C * D, {
                            {16,  8, 4},
                            {36, 16, 8},
                            { 0,  0, 0}
    });
}