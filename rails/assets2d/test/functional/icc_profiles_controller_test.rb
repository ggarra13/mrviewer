require 'test_helper'

class IccProfilesControllerTest < ActionController::TestCase
  setup do
    @icc_profile = icc_profiles(:one)
  end

  test "should get index" do
    get :index
    assert_response :success
    assert_not_nil assigns(:icc_profiles)
  end

  test "should get new" do
    get :new
    assert_response :success
  end

  test "should create icc_profile" do
    assert_difference('IccProfile.count') do
      post :create, icc_profile: {  }
    end

    assert_redirected_to icc_profile_path(assigns(:icc_profile))
  end

  test "should show icc_profile" do
    get :show, id: @icc_profile
    assert_response :success
  end

  test "should get edit" do
    get :edit, id: @icc_profile
    assert_response :success
  end

  test "should update icc_profile" do
    put :update, id: @icc_profile, icc_profile: {  }
    assert_redirected_to icc_profile_path(assigns(:icc_profile))
  end

  test "should destroy icc_profile" do
    assert_difference('IccProfile.count', -1) do
      delete :destroy, id: @icc_profile
    end

    assert_redirected_to icc_profiles_path
  end
end
