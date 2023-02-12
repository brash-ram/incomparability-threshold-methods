#include <iostream>
#include <math.h>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <numeric>
using namespace std;

//Критериальные методы принятия решений: Методы порогов несравнимости
typedef vector<vector<float>> Matrix;

bool more(float a, float b) {
    return a > b;
}

bool equals(float a, float b) {
    return a == b;
}

bool Less(float a, float b) {
    return a < b;
}

long numberOfDigits(double n) {
	std::ostringstream strs;

	strs << n;
	return strs.str().size();
}

void printMatrix(const Matrix& M) {
	long max_len_per_column[M.size()];
	long n = M.size(), m = M[0].size();

	for (long j = 0; j < m; ++j) {
		long max_len {};

		for (long i = 0; i < n; ++i){
			long num_length {numberOfDigits(M[i][j])};
			if (num_length > max_len)
				max_len = num_length;
		}

		max_len_per_column[j] = max_len;
	}

	for (long i = 0; i < n; ++i)
		for (long j = 0; j < m; ++j)
			std::cout << (j == 0 ? "\n| " : "") << std::setw(max_len_per_column[j]) << M[i][j] << (j == m - 1 ? " |" : " ");

	std::cout << '\n';
}

vector<float> getIMatrix(vector<float> a, vector<float> b, bool (*func)(float, float)) {
    vector<float> I;
    for (int i=0; i<a.size(); i++){
        if (func(a[i], b[i])) {
            I.push_back(i);
        }
    }
    return I;
}

Matrix calculateAgreementMatrix(vector<float> A, Matrix X) {
    int sizeX = X.size();
    Matrix agrMatrix(sizeX, vector<float>(sizeX));
    for (int i=0; i<sizeX; i++){
		for (int j=0; j<sizeX; j++){
            int jPlus = (j + i) % sizeX;
            if (i == j) agrMatrix[i][j] = -1;
            if (i == jPlus) continue;
			vector<float> IMore = getIMatrix(X[i], X[jPlus], more);
            vector<float> IEq = getIMatrix  (X[i], X[jPlus], equals);
            vector<float> ILess = getIMatrix(X[i], X[jPlus], Less);
            float C = 0;
            if (!ILess.empty()) {
                for (int k=0; k<A.size(); k++) {
                    if (count(IMore.begin(), IMore.end(), k) > 0 || count(IEq.begin(), IEq.end(), k) > 0) {
                        C += A[k];
                    }
                }
                C /= accumulate(A.begin(), A.end(), 0);
            } else {
                C = 1;
            }
            agrMatrix[jPlus][i] = C;
		}
	}
    return agrMatrix;
}

Matrix calculateDisagreementMatrix(vector<float> L, Matrix X) {
    int sizeX = X.size();
    Matrix disagrMatrix(sizeX, vector<float>(sizeX));
    for (int i=0; i<sizeX; i++){
        for (int j=0; j<sizeX; j++){
            int jPlus = (j + i) % sizeX;
            if (i == j) disagrMatrix[i][j] = -1;
            if (i == jPlus) continue;
            vector<float> ILess = getIMatrix(X[i], X[jPlus], Less);
            vector<float> d;
            for (int k=0; k<ILess.size(); k++){
                d.push_back(abs(X[jPlus][(int) ILess[k]] - X[i][(int) ILess[k]]) / L[(int) ILess[k]]);
            }

            disagrMatrix[jPlus][i] = *max_element(d.begin(), d.end());
        }
    }
    return disagrMatrix;
}

float getLevelAgreement() {
    float C;
    cout << "Enter level agreement (0 <= C <= 1)" << endl;
    cin >> C;
    return C;
}
float getLevelDisagreement() {
    float d;
    cout << "Enter level disagreement (0 <= d <= 1)" << endl;
    cin >> d;
    return d;
}

void printCore(int number, vector<int> core) {
    cout << number << " Core:" << endl;
    for (int i=0; i < core.size(); i++){
        cout << core[i] + 1 << " ";
    }
    cout << endl;
}

vector<int> hardDeleteAlternatives(Matrix& agrMatrix, Matrix& disagrMatrix) {
    float C1 = getLevelAgreement(), d1 = getLevelDisagreement();
    int sizeX = agrMatrix.size();
    vector<int> deletedCriteria;
    for (int i=0; i<sizeX; i++){
        for (int j=0; j<sizeX; j++){
            int jPlus = (j + i) % sizeX;
            if (i == jPlus) continue;

            if (agrMatrix[i][jPlus] >= C1 && disagrMatrix[i][jPlus] >= d1) {
                if (count(deletedCriteria.begin(), deletedCriteria.end(), jPlus) == 0) {
                    deletedCriteria.push_back(jPlus);
                }
            }

        }
    }
    return deletedCriteria;
}

vector<int> softDeleteAlternatives(Matrix& agrMatrix, Matrix& disagrMatrix) {
    float C1 = getLevelAgreement(), d1 = getLevelDisagreement();
    int sizeX = agrMatrix.size();
    vector<int> deletedCriteria;
    for (int i=0; i<sizeX; i++){
        for (int j=0; j<sizeX; j++){
            int jPlus = (j + i) % sizeX;
            if (i == jPlus) continue;

            if (agrMatrix[i][jPlus] >= C1 && disagrMatrix[i][jPlus] <= d1) {
                if (count(deletedCriteria.begin(), deletedCriteria.end(), jPlus) == 0) {
                    deletedCriteria.push_back(jPlus);
                }
            }

        }
    }
    return deletedCriteria;
}

vector<int> compareCores(vector<int> core1, vector<int> core2) {
    vector<int> new_core2;
    for (int i=0; i<core2.size(); i++){
        if (count(core1.begin(), core1.end(), core2[i]) == 0) {
            new_core2.push_back(core2[i]);
        }
    }
    return new_core2;
}

vector<int> createResultCore(vector<int> core1, vector<int> core2, int numberAlternative) {
    vector<int> new_core2;
    for (int i=0; i<numberAlternative; i++){
        if (count(core1.begin(), core1.end(), i) == 0 &&
            count(core2.begin(), core2.end(), i) == 0) {
            new_core2.push_back(i);
        }
    }
    return new_core2;
}

int main() {
    // vector<float> A{3, 2, 1};
    // vector<float> L{100, 50, 45};
    // Matrix X{{180, 70, 10},
    //          {170, 40, 15},
    //          {160, 55, 20},
    //          {150, 50, 25}};
    int numberAlternatives, numberCriteria;

    cout << "Enter number of criterias:" << endl;
    cin >> numberCriteria;
    cout << "Enter number of alternatives:" << endl;
    cin >> numberAlternatives;

    vector<float> A(numberCriteria), L(numberCriteria);
    Matrix X(numberAlternatives, vector<float>(numberCriteria));

    cout << "Enter criteria weights:" << endl;
    for (int i = 0; i < numberCriteria; ++i) {
        cin >> A[i];
    }

    cout << "Enter criteria criteria scale lengths:" << endl;
    for (int i = 0; i < numberCriteria; ++i) {
        cin >> L[i];
    }

    cout << "Enter alternatives:" << endl;
    for (int i = 0; i < numberAlternatives; ++i) {
        for (int j = 0; j < numberCriteria; ++j) {
            cin >> X[i][j];
        }
    }

    cout << "Entred matrix (X):" << endl; 
    printMatrix(X);

    Matrix agrMatrix = calculateAgreementMatrix(A, X);
    Matrix disagrMatrix = calculateDisagreementMatrix(L, X);
    printMatrix(agrMatrix);
    printMatrix(disagrMatrix);
    vector<int> core1 = hardDeleteAlternatives(agrMatrix, disagrMatrix);
    printCore(1, core1);
    vector<int> core2 = softDeleteAlternatives(agrMatrix, disagrMatrix);
    core2 = compareCores(core1, core2);
    printCore(2, core2);
    vector<int> resultCore = createResultCore(core1, core2, X.size());
    cout << "Result ";
    printCore(3, resultCore);

    return 0;
}